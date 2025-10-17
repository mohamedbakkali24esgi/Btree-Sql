#include "btree.h"
#include <stdlib.h>
#include <string.h>

static BTreeNode* node_create(int32_t key, const Row* value) {
    BTreeNode* n = (BTreeNode*)calloc(1, sizeof(BTreeNode));
    if (!n) return NULL;
    n->key = key;
    if (value) n->value = *value;
    return n;
}

static void node_destroy(BTreeNode* n) {
    if (!n) return;
    node_destroy(n->left);
    node_destroy(n->right);
    free(n);
}

BTree* btree_create(void) {
    BTree* t = (BTree*)calloc(1, sizeof(BTree));
    return t;
}

void btree_destroy(BTree* t) {
    if (!t) return;
    node_destroy(t->root);
    free(t);
}

static bool insert_rec(BTreeNode** cur, int32_t key, const Row* value) {
    if (*cur == NULL) {
        *cur = node_create(key, value);
        return *cur != NULL;
    }
    if (key == (*cur)->key) {
        (*cur)->value = *value;
        return true;
    } else if (key < (*cur)->key) {
        return insert_rec(&(*cur)->left, key, value);
    } else {
        return insert_rec(&(*cur)->right, key, value);
    }
}

bool btree_insert(BTree* t, int32_t key, const Row* value) {
    if (!t || !value) return false;
    return insert_rec(&t->root, key, value);
}

static bool search_rec(BTreeNode* cur, int32_t key, Row* out) {
    if (!cur) return false;
    if (key == cur->key) { if (out) *out = cur->value; return true; }
    if (key < cur->key) return search_rec(cur->left, key, out);
    return search_rec(cur->right, key, out);
}

bool btree_search(BTree* t, int32_t key, Row* out) {
    if (!t) return false;
    return search_rec(t->root, key, out);
}

// Simple BST delete (not balanced). You can upgrade to an actual B-Tree later.
static BTreeNode* min_node(BTreeNode* n) {
    while (n && n->left) n = n->left;
    return n;
}

static BTreeNode* delete_rec(BTreeNode* root, int32_t key, bool* removed) {
    if (!root) return NULL;
    if (key < root->key) {
        root->left = delete_rec(root->left, key, removed);
    } else if (key > root->key) {
        root->right = delete_rec(root->right, key, removed);
    } else {
        *removed = true;
        if (!root->left) {
            BTreeNode* r = root->right;
            free(root);
            return r;
        } else if (!root->right) {
            BTreeNode* l = root->left;
            free(root);
            return l;
        } else {
            BTreeNode* succ = min_node(root->right);
            root->key = succ->key;
            root->value = succ->value;
            root->right = delete_rec(root->right, succ->key, removed);
        }
    }
    return root;
}

bool btree_delete(BTree* t, int32_t key) {
    if (!t) return false;
    bool removed = false;
    t->root = delete_rec(t->root, key, &removed);
    return removed;
}

bool btree_update(BTree* tree, const Row* row) {
    if (!tree || !row) return false;
    BTreeNode* node = tree->root;
    while (node) {
        if (row->id < node->value.id) node = node->left;
        else if (row->id > node->value.id) node = node->right;
        else {
            strncpy(node->value.name, row->name, sizeof(node->value.name) - 1);
            node->value.name[sizeof(node->value.name) - 1] = '\0';
            return true;
        }
    }
    return false;
}
