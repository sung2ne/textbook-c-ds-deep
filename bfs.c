// filename: bfs.c
// gcc -std=c17 -O2 -o bfs bfs.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_V 100
#define INF   0x3f3f3f3f

typedef struct Node {
    int to;
    struct Node *next;
} Node;

Node *adj[MAX_V];
int V;

void add_edge(int u, int v) {
    Node *n = malloc(sizeof(Node));
    n->to = v; n->next = adj[u]; adj[u] = n;
    n = malloc(sizeof(Node));
    n->to = u; n->next = adj[v]; adj[v] = n;
}

/* ───────── BFS + 최단 거리 ───────── */
void bfs(int start) {
    int dist[MAX_V];
    int parent[MAX_V];
    int queue[MAX_V];
    int front = 0, back = 0;

    memset(dist, 0x3f, sizeof(dist));  /* INF로 초기화 */
    memset(parent, -1, sizeof(parent));

    dist[start] = 0;
    queue[back++] = start;

    while (front < back) {
        int u = queue[front++];
        printf("방문: %d (거리 %d)\n", u, dist[u]);

        for (Node *cur = adj[u]; cur; cur = cur->next) {
            int v = cur->to;
            if (dist[v] == INF) {
                dist[v] = dist[u] + 1;
                parent[v] = u;
                queue[back++] = v;
            }
        }
    }

    /* 경로 출력: start → target */
    int target = V - 1;
    if (dist[target] == INF) {
        printf("도달 불가\n");
        return;
    }
    printf("\n%d까지 최단 거리: %d\n경로: ", target, dist[target]);
    /* 역추적 */
    int path[MAX_V], plen = 0;
    for (int v = target; v != -1; v = parent[v])
        path[plen++] = v;
    for (int i = plen - 1; i >= 0; i--)
        printf("%d%s", path[i], i ? " -> " : "\n");
}

int main(void) {
    V = 7;
    memset(adj, 0, sizeof(adj));
    add_edge(0, 1); add_edge(0, 2);
    add_edge(1, 3); add_edge(1, 4);
    add_edge(2, 5); add_edge(5, 6);

    printf("=== BFS (시작: 0) ===\n");
    bfs(0);
    return 0;
}
