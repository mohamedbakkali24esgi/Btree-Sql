#include "db.h"
#include "btree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void save_node(FILE* f, BTreeNode* n) {
    if (!n) return;
    save_node(f, n->left);
    fwrite(&n->value, sizeof(Row), 1, f);
    save_node(f, n->right);
}

Table* table_open(const char* filename) {
    Table* t = (Table*)calloc(1, sizeof(Table));
    if (!t) return NULL;

    t->index = btree_create();
    t->filename = filename ? strdup(filename) : strdup("class_db.data");

    // Try to open existing file
    FILE* f = fopen(t->filename, "rb");
    if (f) {
        Row r;
        while (fread(&r, sizeof(Row), 1, f) == 1) {
            btree_insert(t->index, r.id, &r);
        }
        fclose(f);
    }
    return t;
}

void table_close(Table* t) {
    if (!t) return;

    // Save all rows to file
    FILE* f = fopen(t->filename, "wb");
    if (f) {
        save_node(f, t->index->root);
        fclose(f);
    }

    btree_destroy(t->index);
    free((void*)t->filename);
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

