// filename: cache_benchmark.c
// gcc -std=c17 -O2 -o cache_benchmark cache_benchmark.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

#define N (1 << 22)   /* 4M 정수, 약 16MB — L3보다 큼 */
#define REPEAT 8

static inline uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

/* ── 순차 접근 ─────────────────────────────────────────── */
long benchmark_sequential(int *arr, size_t n) {
    long sum = 0;
    for (size_t i = 0; i < n; i++)
        sum += arr[i];
    return sum;
}

/* ── 포인터 체인 (연결 리스트 흉내) ────────────────────── */
typedef struct Node { int value; struct Node *next; } Node;

long benchmark_linked(Node *head) {
    long sum = 0;
    for (Node *p = head; p; p = p->next)
        sum += p->value;
    return sum;
}

/* ── 연결 리스트 노드를 랜덤 순서로 연결 ───────────────── */
Node *build_shuffled_list(size_t n) {
    Node *nodes = malloc(n * sizeof(Node));
    /* Fisher-Yates 셔플로 인덱스 섞기 */
    size_t *idx = malloc(n * sizeof(size_t));
    for (size_t i = 0; i < n; i++) idx[i] = i;
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        size_t tmp = idx[i]; idx[i] = idx[j]; idx[j] = tmp;
    }
    for (size_t i = 0; i < n; i++) nodes[idx[i]].value = (int)i;
    for (size_t i = 0; i < n - 1; i++) nodes[idx[i]].next = &nodes[idx[i + 1]];
    nodes[idx[n - 1]].next = NULL;
    Node *head = &nodes[idx[0]];
    free(idx);
    return head; /* nodes는 해제하지 않음 (벤치마크 후 일괄 해제) */
}

int main(void) {
    srand(42);

    /* 순차 배열 */
    int *arr = malloc(N * sizeof(int));
    for (size_t i = 0; i < N; i++) arr[i] = (int)i;

    /* 연결 리스트 (랜덤 순서) — 노드 수 줄임: L3 부하 목적 */
    size_t list_n = N / 4;   /* 1M 노드, 약 16MB */
    Node *head = build_shuffled_list(list_n);

    /* ── 순차 벤치마크 ── */
    long sum_seq = 0;
    uint64_t t0 = now_ns();
    for (int r = 0; r < REPEAT; r++)
        sum_seq += benchmark_sequential(arr, N);
    uint64_t t1 = now_ns();

    /* ── 연결 리스트 벤치마크 ── */
    long sum_lnk = 0;
    uint64_t t2 = now_ns();
    for (int r = 0; r < REPEAT; r++)
        sum_lnk += benchmark_linked(head);
    uint64_t t3 = now_ns();

    double seq_ms = (t1 - t0) / 1e6 / REPEAT;
    double lnk_ms = (t3 - t2) / 1e6 / REPEAT;

    printf("순차 배열  (%zu개 int):  %.2f ms  (sum=%ld)\n", (size_t)N, seq_ms, sum_seq / REPEAT);
    printf("연결 리스트(%zu개 Node): %.2f ms  (sum=%ld)\n", list_n, lnk_ms, sum_lnk / REPEAT);
    printf("속도비: %.1fx\n", lnk_ms / seq_ms);

    free(arr);
    /* head가 가리키는 nodes 배열은 build_shuffled_list 내부에서 malloc */
    /* 실제 제품 코드라면 해제해야 하지만 벤치마크이므로 생략 */
    return 0;
}
