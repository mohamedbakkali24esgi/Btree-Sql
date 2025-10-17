#ifndef BTREE_H
#define BTREE_H

#include <stdbool.h>
#include <stdint.h>
#include "db.h"

// A deliberately tiny B-Tree-ish API (stubbed).
// You can replace with a real B-Tree or a BST first, then upgrade.

typedef struct BTreeNode {
    int32_t key;
    Row     value;
    struct BTreeNode* left;
    struct BTreeNode* right;
} BTreeNode;

typedef struct BTree {
    BTreeNode* root;
} BTree;

BTree* btree_create(void);
void   btree_destroy(BTree* t);
bool table_update(Table* t, const Row* r);

bool   btree_insert(BTree* t, int32_t key, const Row* value);
bool   btree_search(BTree* t, int32_t key, Row* out);
bool   btree_delete(BTree* t, int32_t key);
bool btree_update(BTree* tree, const Row* row);

#endif // BTREE_H
