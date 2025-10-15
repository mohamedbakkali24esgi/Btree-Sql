#include "db.h"
#include "btree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Table* table_open(const char* filename) {
    Table* t = (Table*)calloc(1, sizeof(Table));
    if (!t) return NULL;
    t->index = btree_create();
    t->filename = filename;

    // TODO: load from disk (persistence)
    // For now, start empty.

    return t;
}

void table_close(Table* t) {
    if (!t) return;
    // TODO: save to disk (persistence)
    btree_destroy(t->index);
    free(t);
}

bool table_insert(Table* t, const Row* r) {
    if (!t || !r) return false;
    return btree_insert(t->index, r->id, r);
}

bool table_select_by_id(Table* t, int32_t id, Row* out) {
    if (!t || !out) return false;
    return btree_search(t->index, id, out);
}

bool table_delete(Table* t, int32_t id) {
    if (!t) return false;
    return btree_delete(t->index, id);
}
