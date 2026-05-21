// filename: bellman_ford.c
// gcc -std=c17 -O2 -o bellman bellman_ford.c

#include <stdio.h>
#include <string.h>

#define MAX_V  100
#define MAX_E  1000
#define INF    0x3f3f3f3f

typedef struct {
    int from, to, weight;
} Edge;

Edge edges[MAX_E];
int E_cnt, V;
int dist[MAX_V];
int parent[MAX_V];

void bellman_ford(int src) {
    memset(dist, 0x3f, sizeof(int) * V);
    memset(parent, -1, sizeof(int) * V);
    dist[src] = 0;

    /* V-1번 반복: 모든 간선 완화 */
    for (int iter = 0; iter < V - 1; iter++) {
        for (int i = 0; i < E_cnt; i++) {
            int u = edges[i].from;
            int v = edges[i].to;
            int w = edges[i].weight;
            if (dist[u] != INF && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;
            }
        }
    }

    /* V번째 반복: 음수 사이클 검출 */
    for (int i = 0; i < E_cnt; i++) {
        int u = edges[i].from;
        int v = edges[i].to;
        int w = edges[i].weight;
        if (dist[u] != INF && dist[u] + w < dist[v]) {
            printf("음수 사이클 존재!\n");
            return;
        }
    }

    printf("0에서 각 노드까지 최단 거리:\n");
    for (int i = 0; i < V; i++) {
        if (dist[i] == INF) printf("  %d: 도달 불가\n", i);
        else printf("  %d: %d\n", i, dist[i]);
    }
}

int main(void) {
    V = 5; E_cnt = 0;

    /* 음수 간선 포함 그래프 */
    edges[E_cnt++] = (Edge){0, 1,  6};
    edges[E_cnt++] = (Edge){0, 2,  7};
    edges[E_cnt++] = (Edge){1, 2,  8};
    edges[E_cnt++] = (Edge){1, 3,  5};
    edges[E_cnt++] = (Edge){1, 4, -4};
    edges[E_cnt++] = (Edge){2, 3, -3};
    edges[E_cnt++] = (Edge){2, 4,  9};
    edges[E_cnt++] = (Edge){3, 1, -2};
    edges[E_cnt++] = (Edge){4, 0,  2};
    edges[E_cnt++] = (Edge){4, 3,  7};

    bellman_ford(0);
    return 0;
}
