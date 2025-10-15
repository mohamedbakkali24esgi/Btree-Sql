#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "db.h"

int main(int argc, char** argv) {
    const char* file = (argc > 1) ? argv[1] : "class_db.data";
    Table* t = table_open(file);
    if (!t) {
        fprintf(stderr, "Failed to open table: %s\n", file);
        return 1;
    }

    printf("class_db ready. Type commands like:\n");
    printf("  INSERT <id> <name>\n");
    printf("  SELECT <id>\n");
    printf("  DELETE <id>\n");
    printf("  .exit\n");

    char line[256];
    while (1) {
        printf("> ");
        if (!fgets(line, sizeof line, stdin)) break;
        if (strncmp(line, ".exit", 5) == 0) break;
        if (!handle_command(t, line)) {
            printf("Unrecognized or failed command.\n");
        }
    }

    table_close(t);
    return 0;
}
