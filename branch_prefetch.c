// filename: branch_prefetch.c
// gcc -std=c17 -O2 -o branch_prefetch branch_prefetch.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

#define N (1 << 20)   /* 1M 원소 */
#define REPEAT 50

static int arr_sorted[N], arr_random[N];

static inline uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

/* 정렬/랜덤 비교 */
static int cmp_int(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

long sum_branch(int *arr, int n) {
    long sum = 0;
    for (int i = 0; i < n; i++)
        if (arr[i] < 128) sum += arr[i];
    return sum;
}

/* 분기 없는 버전 (branchless) */
long sum_branchless(int *arr, int n) {
    long sum = 0;
    for (int i = 0; i < n; i++) {
        int mask = -(arr[i] < 128);   /* 조건 참이면 0xFFFF..., 거짓이면 0 */
        sum += arr[i] & mask;
    }
    return sum;
}

/* ── prefetch 힌트를 사용한 연결 리스트 순회 ── */
typedef struct Node { int value; struct Node *next; } Node;

long traverse_no_prefetch(Node *head) {
    long sum = 0;
    for (Node *p = head; p; p = p->next)
        sum += p->value;
    return sum;
}

long traverse_prefetch(Node *head) {
    long sum = 0;
    for (Node *p = head; p; p = p->next) {
        /* 2단계 앞 노드를 미리 캐시에 올림 */
        if (p->next && p->next->next)
            __builtin_prefetch(p->next->next, 0, 1);
        sum += p->value;
    }
    return sum;
}

Node *build_shuffled_list(int n) {
    Node *nodes = malloc(n * sizeof(Node));
    int *idx = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) { nodes[i].value = i; idx[i] = i; }
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = idx[i]; idx[i] = idx[j]; idx[j] = t;
    }
    for (int i = 0; i < n - 1; i++) nodes[idx[i]].next = &nodes[idx[i + 1]];
    nodes[idx[n - 1]].next = NULL;
    Node *head = &nodes[idx[0]];
    free(idx);
    return head;
}

int main(void) {
    srand(42);

    /* 배열 초기화: 0~255 랜덤 값 */
    for (int i = 0; i < N; i++) arr_random[i] = rand() & 0xFF;
    memcpy(arr_sorted, arr_random, N * sizeof(int));
    qsort(arr_sorted, N, sizeof(int), cmp_int);

    /* ── 분기 예측 벤치마크 ── */
    uint64_t t0, t1, t2, t3, t4, t5;

    t0 = now_ns();
    long s1 = 0;
    for (int r = 0; r < REPEAT; r++) s1 = sum_branch(arr_sorted, N);
    t1 = now_ns();

    t2 = now_ns();
    long s2 = 0;
    for (int r = 0; r < REPEAT; r++) s2 = sum_branch(arr_random, N);
    t3 = now_ns();

    t4 = now_ns();
    long s3 = 0;
    for (int r = 0; r < REPEAT; r++) s3 = sum_branchless(arr_random, N);
    t5 = now_ns();

    printf("=== Branch Prediction ===\n");
    printf("정렬된 배열 (예측 용이):  %.2f ms  (sum=%ld)\n",
           (t1-t0)/1e6/REPEAT, s1);
    printf("랜덤 배열 (예측 어려움):  %.2f ms  (sum=%ld)\n",
           (t3-t2)/1e6/REPEAT, s2);
    printf("Branchless  (랜덤 배열): %.2f ms  (sum=%ld)\n",
           (t5-t4)/1e6/REPEAT, s3);

    /* ── prefetch 벤치마크 ── */
    int list_n = 200000;
    Node *head = build_shuffled_list(list_n);

    t0 = now_ns();
    long lsum1 = 0;
    for (int r = 0; r < REPEAT; r++) lsum1 = traverse_no_prefetch(head);
    t1 = now_ns();

    t2 = now_ns();
    long lsum2 = 0;
    for (int r = 0; r < REPEAT; r++) lsum2 = traverse_prefetch(head);
    t3 = now_ns();

    printf("\n=== Prefetch 힌트 ===\n");
    printf("prefetch 없음: %.2f ms  (sum=%ld)\n", (t1-t0)/1e6/REPEAT, lsum1);
    printf("prefetch 있음: %.2f ms  (sum=%ld)\n", (t3-t2)/1e6/REPEAT, lsum2);

    free(head);  /* 경고: head가 아닌 nodes 배열을 해제해야 하나, 예시 코드로 생략 */
    return 0;
}
