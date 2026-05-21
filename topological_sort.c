// filename: topological_sort.c
// gcc -std=c17 -O2 -o topo topological_sort.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_V 10

typedef struct Node {
    int to;
    struct Node *next;
} Node;

Node *adj[MAX_V];
int visited[MAX_V];
int topo_stack[MAX_V];
int topo_top = -1;

/* WHITE=0, GRAY=1, BLACK=2 */
int color[MAX_V];
int has_cycle = 0;

void add_directed_edge(int u, int v) {
    Node *n = malloc(sizeof(Node));
    n->to = v; n->next = adj[u]; adj[u] = n;
}

/* ───────── 위상정렬 DFS ───────── */
void topo_dfs(int u) {
    color[u] = 1;  /* GRAY: 현재 방문 중 */
    for (Node *cur = adj[u]; cur; cur = cur->next) {
        if (color[cur->to] == 1) {
            has_cycle = 1;  /* 사이클 발견! */
            return;
        }
        if (color[cur->to] == 0)
            topo_dfs(cur->to);
    }
    color[u] = 2;  /* BLACK: 완전히 처리됨 */
    topo_stack[++topo_top] = u;
}

int main(void) {
    int V = 6;
    memset(adj, 0, sizeof(adj));
    memset(color, 0, sizeof(color));

    /* 빌드 의존성: 0→1, 0→2, 1→3, 2→3, 3→4, 4→5 */
    add_directed_edge(0, 1);
    add_directed_edge(0, 2);
    add_directed_edge(1, 3);
    add_directed_edge(2, 3);
    add_directed_edge(3, 4);
    add_directed_edge(4, 5);

    for (int i = 0; i < V; i++)
        if (color[i] == 0)
            topo_dfs(i);

    if (has_cycle) {
        printf("사이클이 있습니다 — 위상정렬 불가\n");
    } else {
        printf("빌드 순서: ");
        while (topo_top >= 0)
            printf("%d ", topo_stack[topo_top--]);
        printf("\n");
    }
    return 0;
}
