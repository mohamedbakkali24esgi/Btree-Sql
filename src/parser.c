#include "db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static char* skip_ws(char* s) {
    while (*s && isspace((unsigned char)*s)) ++s;
    return s;
}

bool handle_command(Table* t, const char* line_in) {
    if (!t || !line_in) return false;
    char buf[256];
    strncpy(buf, line_in, sizeof buf - 1);
    buf[sizeof buf - 1] = '\0';

    char* s = skip_ws(buf);

    if (strncasecmp(s, "INSERT", 6) == 0) {
        s += 6;
        s = skip_ws(s);
        char* endptr = NULL;
        long id = strtol(s, &endptr, 10);
        if (s == endptr) return false;
        s = skip_ws(endptr);
        if (*s == '\0') return false;
        Row r = {0};
        r.id = (int32_t)id;
        // read rest of line as name (trim newline)
        strncpy(r.name, s, sizeof r.name - 1);
        size_t n = strlen(r.name);
        if (n && r.name[n-1] == '\n') r.name[n-1] = '\0';
        if (!table_insert(t, &r)) return false;
        printf("OK\n");
        return true;
    } else if (strncasecmp(s, "SELECT", 6) == 0) {
        s += 6;
        s = skip_ws(s);
        char* endptr = NULL;
        long id = strtol(s, &endptr, 10);
        if (s == endptr) return false;
        Row out = {0};
        if (table_select_by_id(t, (int32_t)id, &out)) {
            printf("%d,%s\n", out.id, out.name);
            return true;
        } else {
            printf("NOT FOUND\n");
            return true;
        }
    } else if (strncasecmp(s, "DELETE", 6) == 0) {
        s += 6;
        s = skip_ws(s);
        char* endptr = NULL;
        long id = strtol(s, &endptr, 10);
        if (s == endptr) return false;
        if (table_delete(t, (int32_t)id)) {
            printf("DELETED\n");
        } else {
            printf("NOT FOUND\n");
        }
        return true;
    }
    return false;
}
