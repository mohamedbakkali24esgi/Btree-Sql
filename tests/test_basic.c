#include <assert.h>
#include <string.h>
#include "db.h"

int main(void) {
    Table* t = table_open(":memory:");
    assert(t);

    Row r = {.id = 1};
    strcpy(r.name, "alice");
    assert(table_insert(t, &r));

    Row out = {0};
    assert(table_select_by_id(t, 1, &out));
    assert(out.id == 1);
    assert(strcmp(out.name, "alice") == 0);

    assert(table_delete(t, 1));
    assert(!table_select_by_id(t, 1, &out));

    table_close(t);
    return 0;
}
