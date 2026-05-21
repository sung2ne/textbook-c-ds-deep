// filename: cache_bench.c
// gcc -std=c17 -O2 -o cache_bench cache_bench.c

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1000000

typedef struct Node {
    int  val;
    struct Node *next;
} Node;

static long long now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

int main(void) {
    /* ─── 배열 합계 ─── */
    int *arr = malloc(sizeof(int) * N);
    for (int i = 0; i < N; i++) arr[i] = i;

    long long t0 = now_ns();
    volatile long long arr_sum = 0;
    for (int i = 0; i < N; i++) arr_sum += arr[i];
    long long t1 = now_ns();
    printf("배열 순회:          %6.2f ms (sum=%lld)\n", (t1-t0)/1e6, arr_sum);

    /* ─── 연결 리스트 합계 ─── */
    Node **nodes = malloc(sizeof(Node *) * N);
    for (int i = 0; i < N; i++) {
        nodes[i] = malloc(sizeof(Node));
        nodes[i]->val  = i;
        nodes[i]->next = NULL;
    }
    /* 랜덤 순서로 연결 (캐시 비친화적 패턴 시뮬레이션) */
    srand(42);
    for (int i = N - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Node *tmp = nodes[i]; nodes[i] = nodes[j]; nodes[j] = tmp;
    }
    for (int i = 0; i < N - 1; i++) nodes[i]->next = nodes[i + 1];

    t0 = now_ns();
    volatile long long list_sum = 0;
    for (Node *cur = nodes[0]; cur; cur = cur->next) list_sum += cur->val;
    t1 = now_ns();
    printf("연결 리스트 순회:   %6.2f ms (sum=%lld)\n", (t1-t0)/1e6, list_sum);

    /* 정리 */
    for (int i = 0; i < N; i++) free(nodes[i]);
    free(nodes);
    free(arr);
    return 0;
}
