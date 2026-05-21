// filename: dijkstra.c
// gcc -std=c17 -O2 -o dijkstra dijkstra.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_V  100
#define MAX_E  10000
#define INF    0x3f3f3f3f

/* ───────── 인접 리스트 ───────── */
typedef struct {
    int to, weight;
} Edge;

Edge edges[MAX_E];
int edge_cnt = 0;
int head[MAX_V];   /* head[u]: u의 첫 번째 간선 인덱스 */
int nxt[MAX_E];    /* nxt[i]: 간선 i 다음 간선 인덱스 */

void add_edge(int u, int v, int w) {
    edges[edge_cnt] = (Edge){v, w};
    nxt[edge_cnt] = head[u];
    head[u] = edge_cnt++;
}

/* ───────── Min Heap (우선순위 큐) ───────── */
typedef struct {
    int dist, node;
} HeapNode;

HeapNode heap[MAX_E];
int heap_size = 0;

void heap_push(int dist, int node) {
    int i = heap_size++;
    heap[i] = (HeapNode){dist, node};
    /* sift up */
    while (i > 0) {
        int p = (i - 1) / 2;
        if (heap[p].dist <= heap[i].dist) break;
        HeapNode tmp = heap[p]; heap[p] = heap[i]; heap[i] = tmp;
        i = p;
    }
}

HeapNode heap_pop(void) {
    HeapNode ret = heap[0];
    heap[0] = heap[--heap_size];
    /* sift down */
    int i = 0;
    while (1) {
        int l = 2*i+1, r = 2*i+2, s = i;
        if (l < heap_size && heap[l].dist < heap[s].dist) s = l;
        if (r < heap_size && heap[r].dist < heap[s].dist) s = r;
        if (s == i) break;
        HeapNode tmp = heap[s]; heap[s] = heap[i]; heap[i] = tmp;
        i = s;
    }
    return ret;
}

/* ───────── 다익스트라 ───────── */
int dist[MAX_V];
int parent[MAX_V];

void dijkstra(int src, int V) {
    memset(dist, 0x3f, sizeof(int) * V);
    memset(parent, -1, sizeof(int) * V);
    heap_size = 0;

    dist[src] = 0;
    heap_push(0, src);

    while (heap_size > 0) {
        HeapNode cur = heap_pop();
        int u = cur.node;
        int d = cur.dist;

        if (d > dist[u]) continue;  /* 이미 더 짧은 경로로 처리됨 */

        for (int i = head[u]; i != -1; i = nxt[i]) {
            int v = edges[i].to;
            int w = edges[i].weight;
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;
                heap_push(dist[v], v);
            }
        }
    }
}

void print_path(int src, int target) {
    if (dist[target] == INF) {
        printf("도달 불가\n");
        return;
    }
    int path[MAX_V], len = 0;
    for (int v = target; v != -1; v = parent[v])
        path[len++] = v;
    printf("최단 거리 %d→%d: %d\n경로: ", src, target, dist[target]);
    for (int i = len - 1; i >= 0; i--)
        printf("%d%s", path[i], i ? " -> " : "\n");
}

int main(void) {
    int V = 6;
    memset(head, -1, sizeof(head));

    /* 가중치 그래프 */
    add_edge(0, 1, 7);  add_edge(0, 2, 9);  add_edge(0, 5, 14);
    add_edge(1, 2, 10); add_edge(1, 3, 15);
    add_edge(2, 3, 11); add_edge(2, 5, 2);
    add_edge(3, 4, 6);
    add_edge(4, 5, 9);

    dijkstra(0, V);

    for (int i = 0; i < V; i++)
        printf("0 → %d: %d\n", i, dist[i] == INF ? -1 : dist[i]);
    printf("\n");
    print_path(0, 4);

    return 0;
}
