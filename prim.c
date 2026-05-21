// filename: prim.c
// gcc -std=c17 -O2 -o prim prim.c

#include <stdio.h>
#include <string.h>

#define MAX_V  20
#define INF    0x3f3f3f3f

int W[MAX_V][MAX_V];  /* 인접 행렬 (가중치) */
int V;

int prim(void) {
    int key[MAX_V];    /* 현재 MST와 연결하는 최소 가중치 */
    int parent[MAX_V]; /* MST에서 부모 노드 */
    int in_mst[MAX_V]; /* MST 포함 여부 */

    memset(key, 0x3f, sizeof(key));
    memset(parent, -1, sizeof(parent));
    memset(in_mst, 0, sizeof(in_mst));

    key[0] = 0;

    for (int cnt = 0; cnt < V; cnt++) {
        /* MST에 없는 노드 중 key 최소인 것 선택 (O(V) 스캔) */
        int u = -1;
        for (int i = 0; i < V; i++)
            if (!in_mst[i] && (u == -1 || key[i] < key[u]))
                u = i;

        in_mst[u] = 1;

        /* u의 이웃 업데이트 */
        for (int v = 0; v < V; v++) {
            if (!in_mst[v] && W[u][v] && W[u][v] < key[v]) {
                key[v] = W[u][v];
                parent[v] = u;
            }
        }
    }

    int total = 0;
    printf("MST 간선:\n");
    for (int i = 1; i < V; i++) {
        printf("  %d - %d (가중치 %d)\n", parent[i], i, key[i]);
        total += key[i];
    }
    return total;
}

int main(void) {
    V = 5;
    memset(W, 0, sizeof(W));

    /* 무향 가중치 그래프 */
    W[0][1] = W[1][0] = 2;
    W[0][3] = W[3][0] = 6;
    W[1][2] = W[2][1] = 3;
    W[1][3] = W[3][1] = 8;
    W[1][4] = W[4][1] = 5;
    W[2][4] = W[4][2] = 7;
    W[3][4] = W[4][3] = 9;

    int total = prim();
    printf("MST 총 가중치: %d\n", total);
    return 0;
}
