// filename: false_sharing.c
// gcc -std=c17 -O2 -lpthread -o false_sharing false_sharing.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

#define THREADS 4
#define ITERS   50000000L

/* ── 나쁜 예: 같은 cache line에 카운터가 밀집 ── */
struct CounterBad {
    long value;   /* 8바이트 */
};
struct CounterBad counters_bad[THREADS];  /* 연속 배치 → 한 cache line에 4개 */

/* ── 좋은 예: 각 카운터를 별도 cache line에 ── */
struct CounterGood {
    long value;           /* 8바이트 */
    char pad[56];         /* 패딩: 64 - 8 = 56바이트 */
};  /* 총 64바이트 = 정확히 1 cache line */
struct CounterGood counters_good[THREADS];

/* 또는 GCC/Clang의 __attribute__((aligned)) 활용 */
struct __attribute__((aligned(64))) CounterAligned {
    long value;
};
struct CounterAligned counters_aligned[THREADS];

static inline uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

void *worker_bad(void *arg) {
    int id = *(int*)arg;
    for (long i = 0; i < ITERS; i++)
        counters_bad[id].value++;
    return NULL;
}

void *worker_good(void *arg) {
    int id = *(int*)arg;
    for (long i = 0; i < ITERS; i++)
        counters_good[id].value++;
    return NULL;
}

void *worker_aligned(void *arg) {
    int id = *(int*)arg;
    for (long i = 0; i < ITERS; i++)
        counters_aligned[id].value++;
    return NULL;
}

void run_bench(void *(*fn)(void*), const char *label) {
    pthread_t threads[THREADS];
    int ids[THREADS];
    for (int i = 0; i < THREADS; i++) ids[i] = i;

    uint64_t t0 = now_ns();
    for (int i = 0; i < THREADS; i++)
        pthread_create(&threads[i], NULL, fn, &ids[i]);
    for (int i = 0; i < THREADS; i++)
        pthread_join(threads[i], NULL);
    uint64_t t1 = now_ns();

    printf("%-25s %.2f ms\n", label, (t1-t0)/1e6);
}

int main(void) {
    printf("sizeof(CounterBad)    = %zu\n", sizeof(struct CounterBad));
    printf("sizeof(CounterGood)   = %zu\n", sizeof(struct CounterGood));
    printf("sizeof(CounterAligned)= %zu\n\n", sizeof(struct CounterAligned));

    printf("counters_bad[0] addr: %p\n", (void*)&counters_bad[0]);
    printf("counters_bad[1] addr: %p  (거리: %zu바이트)\n",
           (void*)&counters_bad[1],
           (size_t)((char*)&counters_bad[1] - (char*)&counters_bad[0]));

    printf("\ncounters_good[0] addr: %p\n", (void*)&counters_good[0]);
    printf("counters_good[1] addr: %p  (거리: %zu바이트)\n\n",
           (void*)&counters_good[1],
           (size_t)((char*)&counters_good[1] - (char*)&counters_good[0]));

    run_bench(worker_bad,     "False Sharing (나쁜 예):");
    run_bench(worker_good,    "Padding 회피 (좋은 예):");
    run_bench(worker_aligned, "alignas(64) 회피:     ");
    return 0;
}
