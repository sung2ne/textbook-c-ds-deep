// filename: tree_traversal.c
// gcc -std=c17 -O2 -o tree_traversal tree_traversal.c

#include <stdio.h>
#include <stdlib.h>

/* ── 노드 구조체 ── */
typedef struct Node {
    int data;
    struct Node *left;
    struct Node *right;
} Node;

/* ── 노드 생성 ── */
Node *new_node(int data) {
    Node *n = malloc(sizeof(Node));
    n->data  = data;
    n->left  = NULL;
    n->right = NULL;
    return n;
}
