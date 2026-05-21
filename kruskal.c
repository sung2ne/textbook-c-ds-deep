// filename: kruskal.c
// gcc -std=c17 -O2 -o kruskal kruskal.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_V  100
#define MAX_E  10000

/* ───────── Union-Find ───────── */
int parent[MAX_V];
int rank_uf[MAX_V];

void uf_init(int n) {
    for (int i = 0; i < n; i++) { parent[i] = i; rank_uf[i] = 0; }
}

int uf_find(int x) {
    /* 경로 압축(path compression) */
    if (parent[x] != x)
        parent[x] = uf_find(parent[x]);
    return parent[x];
}

int uf_union(int x, int y) {
    int px = uf_find(x), py = uf_find(y);
    if (px == py) return 0;  /* 같은 집합 → 사이클 */
    /* 랭크에 의한 합치기(union by rank) */
    if (rank_uf[px] < rank_uf[py]) { int t = px; px = py; py = t; }
    parent[py] = px;
    if (rank_uf[px] == rank_uf[py]) rank_uf[px]++;
    return 1;
}

/* ───────── 크루스칼 ───────── */
typedef struct {
    int u, v, w;
} Edge;

Edge edges[MAX_E];
int E_cnt;

int edge_cmp(const void *a, const void *b) {
    return ((Edge*)a)->w - ((Edge*)b)->w;
}

int kruskal(int V) {
    qsort(edges, E_cnt, sizeof(Edge), edge_cmp);
    uf_init(V);

    int total = 0, selected = 0;
    printf("선택된 간선:\n");
    for (int i = 0; i < E_cnt && selected < V - 1; i++) {
        Edge e = edges[i];
        if (uf_union(e.u, e.v)) {
            printf("  %d - %d (가중치 %d)\n", e.u, e.v, e.w);
            total += e.w;
            selected++;
        }
    }
    return total;
}

int main(void) {
    int V = 7; E_cnt = 0;
    edges[E_cnt++] = (Edge){0, 1, 7};
    edges[E_cnt++] = (Edge){0, 3, 5};
    edges[E_cnt++] = (Edge){1, 2, 8};
    edges[E_cnt++] = (Edge){1, 3, 9};
    edges[E_cnt++] = (Edge){1, 4, 7};
    edges[E_cnt++] = (Edge){2, 4, 5};
    edges[E_cnt++] = (Edge){3, 4, 15};
    edges[E_cnt++] = (Edge){3, 5, 6};
    edges[E_cnt++] = (Edge){4, 5, 8};
    edges[E_cnt++] = (Edge){4, 6, 9};
    edges[E_cnt++] = (Edge){5, 6, 11};

    int mst_cost = kruskal(V);
    printf("MST 총 가중치: %d\n", mst_cost);
    return 0;
}
