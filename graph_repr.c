// filename: graph_repr.c
// gcc -std=c17 -O2 -o graph_repr graph_repr.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_V 100

/* ───────── 인접 행렬 방식 ───────── */
typedef struct {
    int adj[MAX_V][MAX_V];
    int V;
} MatrixGraph;

void matrix_init(MatrixGraph *g, int v) {
    g->V = v;
    memset(g->adj, 0, sizeof(g->adj));
}

void matrix_add_edge(MatrixGraph *g, int u, int v) {
    g->adj[u][v] = 1;
    g->adj[v][u] = 1;  // 무향 그래프
}

int matrix_has_edge(MatrixGraph *g, int u, int v) {
    return g->adj[u][v];
}

/* ───────── 인접 리스트 방식 ───────── */
typedef struct Node {
    int to;
    int weight;
    struct Node *next;
} Node;

typedef struct {
    Node *adj[MAX_V];
    int V;
} ListGraph;

void list_init(ListGraph *g, int v) {
    g->V = v;
    for (int i = 0; i < v; i++) g->adj[i] = NULL;
}

void list_add_edge(ListGraph *g, int u, int v, int w) {
    /* u → v */
    Node *node = malloc(sizeof(Node));
    node->to = v;
    node->weight = w;
    node->next = g->adj[u];
    g->adj[u] = node;

    /* v → u (무향 그래프) */
    node = malloc(sizeof(Node));
    node->to = u;
    node->weight = w;
    node->next = g->adj[v];
    g->adj[v] = node;
}

void list_print(ListGraph *g) {
    for (int i = 0; i < g->V; i++) {
        printf("%d:", i);
        for (Node *cur = g->adj[i]; cur; cur = cur->next)
            printf(" -> %d(w=%d)", cur->to, cur->weight);
        printf("\n");
    }
}

void list_free(ListGraph *g) {
    for (int i = 0; i < g->V; i++) {
        Node *cur = g->adj[i];
        while (cur) {
            Node *tmp = cur->next;
            free(cur);
            cur = tmp;
        }
    }
}

int main(void) {
    printf("=== 인접 행렬 ===\n");
    MatrixGraph mg;
    matrix_init(&mg, 5);
    matrix_add_edge(&mg, 0, 1);
    matrix_add_edge(&mg, 0, 2);
    matrix_add_edge(&mg, 1, 3);
    matrix_add_edge(&mg, 2, 4);

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++)
            printf("%d ", mg.adj[i][j]);
        printf("\n");
    }
    printf("엣지 0-1 존재: %s\n", matrix_has_edge(&mg, 0, 1) ? "Yes" : "No");
    printf("엣지 1-4 존재: %s\n", matrix_has_edge(&mg, 1, 4) ? "Yes" : "No");

    printf("\n=== 인접 리스트 (가중치 그래프) ===\n");
    ListGraph lg;
    list_init(&lg, 5);
    list_add_edge(&lg, 0, 1, 4);
    list_add_edge(&lg, 0, 2, 2);
    list_add_edge(&lg, 1, 3, 5);
    list_add_edge(&lg, 2, 4, 3);
    list_print(&lg);
    list_free(&lg);

    return 0;
}
