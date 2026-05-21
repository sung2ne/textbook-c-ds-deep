// filename: bipartite_check.c
// gcc -std=c17 -O2 -o bipartite bipartite_check.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_V 100

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

int is_bipartite(void) {
    int color[MAX_V];
    int queue[MAX_V];
    memset(color, -1, sizeof(color));

    for (int s = 0; s < V; s++) {
        if (color[s] != -1) continue;
        int front = 0, back = 0;
        color[s] = 0;
        queue[back++] = s;

        while (front < back) {
            int u = queue[front++];
            for (Node *cur = adj[u]; cur; cur = cur->next) {
                int v = cur->to;
                if (color[v] == -1) {
                    color[v] = 1 - color[u];  /* 반대 색 */
                    queue[back++] = v;
                } else if (color[v] == color[u]) {
                    return 0;  /* 같은 색 이웃 → 이분 그래프 아님 */
                }
            }
        }
    }
    return 1;
}

int main(void) {
    V = 4;
    memset(adj, 0, sizeof(adj));

    /* 사각형 그래프: 이분 그래프 */
    add_edge(0, 1); add_edge(1, 2);
    add_edge(2, 3); add_edge(3, 0);
    printf("사각형(이분?): %s\n", is_bipartite() ? "Yes" : "No");

    /* 삼각형: 이분 그래프 아님 */
    V = 3;
    for (int i = 0; i < V; i++) adj[i] = NULL;
    add_edge(0, 1); add_edge(1, 2); add_edge(2, 0);
    printf("삼각형(이분?): %s\n", is_bipartite() ? "Yes" : "No");

    return 0;
}
