// filename: make_deps.c
// gcc -std=c17 -O2 -o make_deps make_deps.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_V 10

const char *module_name[] = {
    "libc", "libm", "libssl", "libcrypto",
    "utils", "network", "parser", "main", "test", "config"
};

typedef struct Node {
    int to;
    struct Node *next;
} Node;

Node *adj[MAX_V];
int in_degree[MAX_V];
int V;

void add_dep(int from, int to) {
    /* from은 to에 의존 → to를 먼저 빌드해야 함 */
    /* 그래프: to → from */
    Node *n = malloc(sizeof(Node));
    n->to = from; n->next = adj[to]; adj[to] = n;
    in_degree[from]++;
}

int kahn_topo(int *result) {
    int queue[MAX_V];
    int front = 0, back = 0;

    /* 진입 차수가 0인 노드부터 시작 */
    for (int i = 0; i < V; i++)
        if (in_degree[i] == 0)
            queue[back++] = i;

    int count = 0;
    while (front < back) {
        int u = queue[front++];
        result[count++] = u;

        for (Node *cur = adj[u]; cur; cur = cur->next) {
            if (--in_degree[cur->to] == 0)
                queue[back++] = cur->to;
        }
    }

    return count;  /* V와 다르면 사이클 존재 */
}

int main(void) {
    V = 8;
    memset(adj, 0, sizeof(adj));
    memset(in_degree, 0, sizeof(in_degree));

    /* 의존성 정의 (A는 B에 의존) */
    add_dep(7, 6);  /* main <- parser */
    add_dep(7, 5);  /* main <- network */
    add_dep(7, 9);  /* main <- config (index 9 없음, V=8로 맞춤) */
    add_dep(6, 4);  /* parser <- utils */
    add_dep(5, 2);  /* network <- libssl */
    add_dep(5, 4);  /* network <- utils */
    add_dep(2, 3);  /* libssl <- libcrypto */
    add_dep(4, 0);  /* utils <- libc */
    add_dep(3, 0);  /* libcrypto <- libc */
    add_dep(1, 0);  /* libm <- libc */

    int result[MAX_V];
    int cnt = kahn_topo(result);

    if (cnt != V) {
        printf("오류: 순환 의존성이 있습니다! 빌드 불가.\n");
    } else {
        printf("빌드 순서:\n");
        for (int i = 0; i < cnt; i++)
            printf("  %d. %s\n", i + 1, module_name[result[i]]);
    }

    return 0;
}
