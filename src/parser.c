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

// Helper: print all rows recursively
static void print_all_rows(BTreeNode* node) {
    if (!node) return;
    print_all_rows(node->left);
    printf("%d,'%s'\n", node->value.id, node->value.name);
    print_all_rows(node->right);
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
        // Check if already exists → reject
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

        // SELECT * FROM table;
        if (*s == '\0') {
            print_all_rows(t->index->root);
            return true;
        }

        if (strncasecmp(s, "WHERE", 5) != 0) return false;
        s += 5; s = skip_ws(s);

        // support WHERE id=...  OR  WHERE name='...'
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
            // brute-force scan all
            // print matches by name
            BTreeNode* node = t->index->root;
            // define stack manually (simple DFS)
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

    return false;
}

