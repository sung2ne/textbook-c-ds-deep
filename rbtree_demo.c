// filename: rbtree_demo.c
// gcc -std=c17 -O2 -o rbtree_demo rbtree_demo.c

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>   /* offsetof */

/* ── 색상 상수 ── */
#define RB_RED   0
#define RB_BLACK 1

/* ── 노드 구조체 ── */
typedef struct RBNode {
    struct RBNode *parent;
    struct RBNode *left;
    struct RBNode *right;
    int            color;
} RBNode;

/* ── container_of 매크로 ── */
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

/* ── 사용자 데이터 구조체 — rb_node를 embed ── */
typedef struct MyData {
    int    key;
    char   value[64];
    RBNode rb;               /* ← RB Tree 노드 embed */
} MyData;

/* ── rb_entry: RBNode* → MyData* ── */
static MyData *rb_entry(RBNode *node) {
    return container_of(node, MyData, rb);
}

/* ── 트리 루트 ── */
typedef struct {
    RBNode *root;
} RBTree;

static void rbtree_init(RBTree *t) { t->root = NULL; }

/* ── 탐색 ── */
MyData *rbtree_search(RBTree *t, int key) {
    RBNode *cur = t->root;
    while (cur) {
        MyData *d = rb_entry(cur);
        if      (key < d->key) cur = cur->left;
        else if (key > d->key) cur = cur->right;
        else                   return d;
    }
    return NULL;
}

/* ── 색상 수정 없는 단순 BST 삽입 (데모용) ── */
void rbtree_insert_simple(RBTree *t, MyData *data) {
    RBNode **pos = &t->root;
    RBNode  *par = NULL;

    while (*pos) {
        MyData *d = rb_entry(*pos);
        par = *pos;
        if      (data->key < d->key) pos = &(*pos)->left;
        else if (data->key > d->key) pos = &(*pos)->right;
        else return;   /* 중복 */
    }
    data->rb.parent = par;
    data->rb.left   = NULL;
    data->rb.right  = NULL;
    data->rb.color  = RB_RED;
    *pos = &data->rb;
}

int main(void) {
    RBTree tree;
    rbtree_init(&tree);

    /* 데이터 삽입 */
    int keys[] = {50, 30, 70, 20, 40, 60, 80};
    MyData nodes[7];
    for (int i = 0; i < 7; i++) {
        nodes[i].key = keys[i];
        snprintf(nodes[i].value, sizeof(nodes[i].value),
                 "task_%d", keys[i]);
        rbtree_insert_simple(&tree, &nodes[i]);
    }

    /* 탐색 */
    int search_key = 40;
    MyData *found = rbtree_search(&tree, search_key);
    if (found)
        printf("찾음: key=%d, value=%s\n", found->key, found->value);
    else
        printf("key=%d 없음\n", search_key);

    return 0;
}
