// filename: floyd_warshall.c
// gcc -std=c17 -O2 -o floyd floyd_warshall.c

#include <stdio.h>
#include <string.h>

#define MAX_V  10
#define INF    0x3f3f3f3f

int dist[MAX_V][MAX_V];
int nxt[MAX_V][MAX_V];   /* 경로 추적용 */
int V;

void floyd_warshall(void) {
    /* 경로 추적 초기화 */
    for (int i = 0; i < V; i++)
        for (int j = 0; j < V; j++)
            nxt[i][j] = (dist[i][j] != INF && i != j) ? j : -1;

    /* 메인 DP */
    for (int k = 0; k < V; k++) {
        for (int i = 0; i < V; i++) {
            for (int j = 0; j < V; j++) {
                if (dist[i][k] == INF || dist[k][j] == INF) continue;
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    nxt[i][j] = nxt[i][k];
                }
            }
        }
    }

    /* 음수 사이클 검출: 대각선이 음수이면 사이클 */
    for (int i = 0; i < V; i++)
        if (dist[i][i] < 0) {
            printf("음수 사이클 존재!\n");
            return;
        }
}

void print_path(int u, int v) {
    if (nxt[u][v] == -1) { printf("경로 없음"); return; }
    printf("%d", u);
    while (u != v) {
        u = nxt[u][v];
        printf(" -> %d", u);
    }
}

int main(void) {
    V = 4;
    /* INF로 초기화, 자기 자신 = 0 */
    memset(dist, 0x3f, sizeof(dist));
    for (int i = 0; i < V; i++) dist[i][i] = 0;

    /* 간선 추가 */
    dist[0][1] = 3; dist[0][3] = 7;
    dist[1][0] = 8; dist[1][2] = 2;
    dist[2][0] = 5; dist[2][3] = 1;
    dist[3][0] = 2;

    floyd_warshall();

    printf("모든 쌍 최단 거리:\n");
    for (int i = 0; i < V; i++) {
        for (int j = 0; j < V; j++) {
            if (dist[i][j] == INF) printf("  INF");
            else printf("%5d", dist[i][j]);
        }
        printf("\n");
    }

    printf("\n경로 0→3: ");
    print_path(0, 3);
    printf("\n");

    return 0;
}
