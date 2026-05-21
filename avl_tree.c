// filename: avl_tree.c
// gcc -std=c17 -O2 -o avl_tree avl_tree.c

#include <stdio.h>
#include <stdlib.h>

typedef struct AVLNode {
    int            data;
    int            height;
    struct AVLNode *left;
    struct AVLNode *right;
} AVLNode;

/* ── 유틸리티 ── */
static int max(int a, int b) { return a > b ? a : b; }

static int height(const AVLNode *n) {
    return n ? n->height : -1;   /* NULL의 높이는 -1 */
}

static void update_height(AVLNode *n) {
    n->height = 1 + max(height(n->left), height(n->right));
}

static int balance_factor(const AVLNode *n) {
    return height(n->left) - height(n->right);
}

static AVLNode *new_node(int data) {
    AVLNode *n  = malloc(sizeof(AVLNode));
    n->data     = data;
    n->height   = 0;
    n->left     = NULL;
    n->right    = NULL;
    return n;
}

/* ── LL 회전 (오른쪽으로 회전) ── */
static AVLNode *rotate_right(AVLNode *z) {
    AVLNode *y  = z->left;
    AVLNode *T3 = y->right;

    y->right = z;
    z->left  = T3;

    update_height(z);
    update_height(y);
    return y;   /* 새 루트 */
}

/* ── RR 회전 (왼쪽으로 회전) ── */
static AVLNode *rotate_left(AVLNode *z) {
    AVLNode *y  = z->right;
    AVLNode *T2 = y->left;

    y->left  = z;
    z->right = T2;

    update_height(z);
    update_height(y);
    return y;   /* 새 루트 */
}

/* ── 불균형 수정 ── */
static AVLNode *rebalance(AVLNode *n) {
    update_height(n);
    int bf = balance_factor(n);

    /* LL Case */
    if (bf > 1 && balance_factor(n->left) >= 0)
        return rotate_right(n);

    /* LR Case */
    if (bf > 1 && balance_factor(n->left) < 0) {
        n->left = rotate_left(n->left);
        return rotate_right(n);
    }

    /* RR Case */
    if (bf < -1 && balance_factor(n->right) <= 0)
        return rotate_left(n);

    /* RL Case */
    if (bf < -1 && balance_factor(n->right) > 0) {
        n->right = rotate_right(n->right);
        return rotate_left(n);
    }

    return n;
}

/* ── AVL 삽입 ── */
AVLNode *avl_insert(AVLNode *root, int data) {
    if (root == NULL) return new_node(data);

    if (data < root->data)
        root->left  = avl_insert(root->left,  data);
    else if (data > root->data)
        root->right = avl_insert(root->right, data);
    else
        return root;  /* 중복 무시 */

    return rebalance(root);
}

static void inorder(const AVLNode *root) {
    if (!root) return;
    inorder(root->left);
    printf("%d(h=%d) ", root->data, root->height);
    inorder(root->right);
}

static void free_tree(AVLNode *root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

int main(void) {
    AVLNode *root = NULL;
    int keys[] = {1, 2, 3, 4, 5, 6, 7};

    for (int i = 0; i < 7; i++)
        root = avl_insert(root, keys[i]);

    printf("AVL 중위 순회: ");
    inorder(root);
    printf("\n");
    printf("루트: %d, 높이: %d\n", root->data, root->height);

    free_tree(root);
    return 0;
}
