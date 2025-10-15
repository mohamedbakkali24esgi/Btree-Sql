#ifndef DB_H
#define DB_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// A very small "row" representation: fixed-size fields for simplicity.
typedef struct {
    int32_t id;
    char    name[64];
} Row;

// A table is backed by a B-Tree (declared in btree.h).
struct BTree;
typedef struct {
    struct BTree* index;
    const char*   filename; // persistence target
} Table;

Table* table_open(const char* filename);
void   table_close(Table* t);
bool   table_insert(Table* t, const Row* r);
bool   table_select_by_id(Table* t, int32_t id, Row* out);
bool   table_delete(Table* t, int32_t id);

// Very tiny text protocol parser (INSERT/SELECT)
bool   handle_command(Table* t, const char* line);

#endif // DB_H
