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
    printf("  INSERT INTO table VALUES ('id','name')\n");
    printf("  SELECT * FROM table WHERE id=*\n");
    printf("  SELECT * FROM table \n");
    printf("  UPDATE table SET name='newname' WHERE id=*");
    printf("  DELETE 'id' WHERE id=*\n");
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
