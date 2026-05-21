// filename: dfs.c
// gcc -std=c17 -O2 -o dfs dfs.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_V 100

typedef struct Node {
    int to;
    struct Node *next;
} Node;

Node *adj[MAX_V];
int visited[MAX_V];
int V;

void add_edge(int u, int v) {
    Node *n = malloc(sizeof(Node));
    n->to = v; n->next = adj[u]; adj[u] = n;
    n = malloc(sizeof(Node));
    n->to = u; n->next = adj[v]; adj[v] = n;
}

/* ───────── 재귀 DFS ───────── */
void dfs(int u) {
    visited[u] = 1;
    printf("%d ", u);
    for (Node *cur = adj[u]; cur; cur = cur->next) {
        if (!visited[cur->to])
            dfs(cur->to);
    }
}

/* ───────── 반복 DFS (명시적 스택) ───────── */
void dfs_iterative(int start) {
    int stack[MAX_V], top = -1;
    int vis[MAX_V] = {0};

    stack[++top] = start;
    while (top >= 0) {
        int u = stack[top--];
        if (vis[u]) continue;
        vis[u] = 1;
        printf("%d ", u);
        for (Node *cur = adj[u]; cur; cur = cur->next)
            if (!vis[cur->to])
                stack[++top] = cur->to;
    }
}

int main(void) {
    V = 6;
    memset(adj, 0, sizeof(adj));
    memset(visited, 0, sizeof(visited));

    add_edge(0, 1); add_edge(0, 2);
    add_edge(1, 3); add_edge(1, 4);
    add_edge(2, 5);

    printf("재귀 DFS: ");
    dfs(0);
    printf("\n반복 DFS: ");
    dfs_iterative(0);
    printf("\n");
    return 0;
}
