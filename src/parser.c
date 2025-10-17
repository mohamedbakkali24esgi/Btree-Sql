#include "db.h"
#include "btree.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static char* skip_ws(char* s) {
    while (*s && isspace((unsigned char)*s)) ++s;
    return s;
}

static void trim_trailing_ws(char* s) {
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) s[--n] = '\0';
}

// Parse quoted or plain value
static char* parse_value(char** ps, char* out, size_t outsz) {
    char* s = skip_ws(*ps);
    if (*s == '\'' || *s == '"') {
        char quote = *s++;
        size_t i = 0;
        while (*s && *s != quote && i + 1 < outsz) out[i++] = *s++;
        out[i] = '\0';
        if (*s == quote) s++;
    } else {
        size_t i = 0;
        while (*s && *s != ',' && *s != ')' && !isspace((unsigned char)*s) && i + 1 < outsz)
            out[i++] = *s++;
        out[i] = '\0';
    }
    *ps = skip_ws(s);
    return out;
}

// Comparison functions for sorting
static int compare_by_id_asc(const void* a, const void* b) {
    return ((Row*)a)->id - ((Row*)b)->id;
}

static int compare_by_id_desc(const void* a, const void* b) {
    return ((Row*)b)->id - ((Row*)a)->id;
}

static int compare_by_name_asc(const void* a, const void* b) {
    return strcasecmp(((Row*)a)->name, ((Row*)b)->name);
}

static int compare_by_name_desc(const void* a, const void* b) {
    return strcasecmp(((Row*)b)->name, ((Row*)a)->name);
}

// Collect rows into an array
static void collect_rows(BTreeNode* node, Row* rows, int* count, int max_count) {
    if (!node || *count >= max_count) return;
    collect_rows(node->left, rows, count, max_count);
    if (*count < max_count) {
        rows[*count] = node->value;
        (*count)++;
    }
    collect_rows(node->right, rows, count, max_count);
}

// Print rows from array
static void print_rows(Row* rows, int count) {
    for (int i = 0; i < count; i++) {
        printf("%d,'%s'\n", rows[i].id, rows[i].name);
    }
}

bool handle_command(Table* t, const char* line_in) {
    if (!t || !line_in) return false;
    char buf[512];
    strncpy(buf, line_in, sizeof buf - 1);
    buf[sizeof buf - 1] = '\0';
    trim_trailing_ws(buf);
    char* s = skip_ws(buf);

    // =========== INSERT =====================
    if (strncasecmp(s, "INSERT", 6) == 0) {
        s += 6; s = skip_ws(s);
        if (strncasecmp(s, "INTO", 4) == 0) {
            s += 4; s = skip_ws(s);
            while (*s && !isspace((unsigned char)*s)) ++s;
            s = skip_ws(s);
        }
        if (strncasecmp(s, "VALUES", 6) != 0) return false;
        s += 6; s = skip_ws(s);
        if (*s != '(') return false;
        s++; s = skip_ws(s);

        char id_str[32], name[64];
        parse_value(&s, id_str, sizeof id_str);
        if (*s == ',') s++;
        parse_value(&s, name, sizeof name);
        if (*s == ')') s++;

        int32_t id = (int32_t)atoi(id_str);
        Row check;
        if (table_select_by_id(t, id, &check)) {
            printf("ERROR: duplicate primary key id=%d\n", id);
            return true;
        }

        Row r = { .id = id };
        strncpy(r.name, name, sizeof r.name - 1);
        if (!table_insert(t, &r)) {
            printf("Insert failed\n");
            return false;
        }
        printf("OK\n");
        return true;
    }

    // =========== SELECT =====================
    if (strncasecmp(s, "SELECT", 6) == 0) {
        s += 6; s = skip_ws(s);
        if (strncmp(s, "*", 1) == 0) s++;
        s = skip_ws(s);
        if (strncasecmp(s, "FROM", 4) == 0) {
            s += 4; s = skip_ws(s);
            while (*s && !isspace((unsigned char)*s)) ++s;
            s = skip_ws(s);
        }

        // Handle WHERE clause
        if (strncasecmp(s, "WHERE", 5) == 0) {
            s += 5; s = skip_ws(s);
            if (strncasecmp(s, "id", 2) == 0) {
                s += 2; s = skip_ws(s);
                if (*s != '=') return false;
                s++; s = skip_ws(s);
                char id_str[32]; parse_value(&s, id_str, sizeof id_str);
                Row out = {0};
                if (table_select_by_id(t, (int32_t)atoi(id_str), &out))
                    printf("%d,'%s'\n", out.id, out.name);
                else
                    printf("NOT FOUND\n");
                return true;
            }

            if (strncasecmp(s, "name", 4) == 0) {
                s += 4; s = skip_ws(s);
                if (*s != '=') return false;
                s++; s = skip_ws(s);
                char name[64]; parse_value(&s, name, sizeof name);
                BTreeNode* node = t->index->root;
                BTreeNode* stack[256];
                int top = 0;
                while (node || top) {
                    while (node) {
                        stack[top++] = node;
                        node = node->left;
                    }
                    node = stack[--top];
                    if (strcasecmp(node->value.name, name) == 0)
                        printf("%d,'%s'\n", node->value.id, node->value.name);
                    node = node->right;
                }
                return true;
            }
            return false;
        }

        // Handle ORDER BY clause
        Row rows[256]; // Assume max 256 rows for simplicity
        int count = 0;
        collect_rows(t->index->root, rows, &count, 256);

        if (strncasecmp(s, "ORDER BY", 8) == 0) {
            s += 8; s = skip_ws(s);
            bool by_id = false;
            if (strncasecmp(s, "id", 2) == 0) {
                by_id = true;
                s += 2;
            } else if (strncasecmp(s, "name", 4) == 0) {
                by_id = false;
                s += 4;
            } else {
                return false;
            }
            s = skip_ws(s);

            bool ascending = true;
            if (strncasecmp(s, "DESC", 4) == 0) {
                ascending = false;
                s += 4;
            } else if (strncasecmp(s, "ASC", 3) == 0) {
                s += 3;
            }
            s = skip_ws(s);
            if (*s != '\0' && *s != ';') return false;

            // Sort rows
            if (by_id) {
                qsort(rows, count, sizeof(Row), ascending ? compare_by_id_asc : compare_by_id_desc);
            } else {
                qsort(rows, count, sizeof(Row), ascending ? compare_by_name_asc : compare_by_name_desc);
            }
        } else {
            // Default: sort by id ascending (matches original in-order traversal)
            qsort(rows, count, sizeof(Row), compare_by_id_asc);
        }

        print_rows(rows, count);
        return true;
    }

    // =========== DELETE =====================
    if (strncasecmp(s, "DELETE", 6) == 0) {
        s += 6; s = skip_ws(s);
        if (strncasecmp(s, "FROM", 4) == 0) {
            s += 4; s = skip_ws(s);
            while (*s && !isspace((unsigned char)*s)) ++s;
            s = skip_ws(s);
        }
        if (strncasecmp(s, "WHERE", 5) != 0) return false;
        s += 5; s = skip_ws(s);
        if (strncasecmp(s, "id", 2) != 0) return false;
        s += 2; s = skip_ws(s);
        if (*s != '=') return false;
        s++; s = skip_ws(s);
        char id_str[32]; parse_value(&s, id_str, sizeof id_str);
        if (table_delete(t, (int32_t)atoi(id_str)))
            printf("DELETED\n");
        else
            printf("NOT FOUND\n");
        return true;
    }

    // =========== UPDATE =====================
    if (strncasecmp(s, "UPDATE", 6) == 0) {
        s += 6; s = skip_ws(s);
        while (*s && !isspace((unsigned char)*s)) ++s;
        s = skip_ws(s);

        if (strncasecmp(s, "SET", 3) != 0) return false;
        s += 3; s = skip_ws(s);

        if (strncasecmp(s, "name", 4) != 0) return false;
        s += 4; s = skip_ws(s);

        if (*s != '=') return false;
        s++; s = skip_ws(s);

        char newname[64];
        parse_value(&s, newname, sizeof newname);
        s = skip_ws(s);

        if (strncasecmp(s, "WHERE", 5) != 0) return false;
        s += 5; s = skip_ws(s);

        if (strncasecmp(s, "id", 2) != 0) return false;
        s += 2; s = skip_ws(s);

        if (*s != '=') return false;
        s++; s = skip_ws(s);

        char id_str[32];
        parse_value(&s, id_str, sizeof id_str);

        int32_t id = (int32_t)atoi(id_str);
        Row r;
        if (!table_select_by_id(t, id, &r)) {
            printf("NOT FOUND\n");
            return true;
        }

        strncpy(r.name, newname, sizeof r.name - 1);
        r.name[sizeof r.name - 1] = '\0';
        if (!table_update(t, &r)) {
            printf("UPDATE FAILED\n");
            return false;
        }

        printf("UPDATED\n");
        return true;
    }

    return false;
}