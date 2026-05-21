// filename: atomics_basics.c
// gcc -std=c17 -O2 -lpthread -o atomics_basics atomics_basics.c
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>

#define THREADS 8
#define ITERS   1000000L

/* ── 1. 원자적 카운터 ───────────────────────── */
_Atomic long atomic_counter = 0;
long         plain_counter   = 0;   /* 비교용: 버그 있는 버전 */

void *inc_atomic(void *arg) {
    (void)arg;
    for (long i = 0; i < ITERS; i++)
        atomic_fetch_add(&atomic_counter, 1);
    return NULL;
}

void *inc_plain(void *arg) {
    (void)arg;
    for (long i = 0; i < ITERS; i++)
        plain_counter++;   /* race condition! */
    return NULL;
}

/* ── 2. atomic_compare_exchange 예제 ────────── */
/* lock-free max 업데이트: current_max < value 이면 교체 */
_Atomic int current_max = 0;

void atomic_update_max(int value) {
    int expected = atomic_load(&current_max);
    while (value > expected) {
        /* 성공하면 current_max = value, 실패하면 expected가 갱신됨 */
        if (atomic_compare_exchange_weak(&current_max, &expected, value))
            break;
        /* 실패: expected에 최신 값이 들어옴, 루프 재시도 */
    }
}

/* ── 3. atomic_exchange 예제 (spinlock) ──────── */
_Atomic int spinlock = 0;   /* 0=열림, 1=잠김 */

void spin_lock(void) {
    int zero = 0;
    /* 0 → 1로 교체 성공할 때까지 반복 */
    while (!atomic_compare_exchange_weak(&spinlock, &zero, 1))
        zero = 0;   /* 실패 시 expected 리셋 */
}

void spin_unlock(void) {
    atomic_store(&spinlock, 0);
}

_Atomic int protected_value = 0;

void *spinlock_worker(void *arg) {
    (void)arg;
    for (int i = 0; i < 100000; i++) {
        spin_lock();
        protected_value++;   /* 임계 구역 */
        spin_unlock();
    }
    return NULL;
}

static inline uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int main(void) {
    pthread_t tids[THREADS];

    /* ── 원자적 카운터 테스트 ── */
    for (int i = 0; i < THREADS; i++)
        pthread_create(&tids[i], NULL, inc_atomic, NULL);
    for (int i = 0; i < THREADS; i++) pthread_join(tids[i], NULL);
    printf("원자적 카운터 결과: %ld (기대: %ld) %s\n",
           atomic_load(&atomic_counter), (long)THREADS * ITERS,
           atomic_counter == THREADS * ITERS ? "✓" : "✗ 버그");

    /* ── plain 카운터 테스트 (버그 재현) ── */
    for (int i = 0; i < THREADS; i++)
        pthread_create(&tids[i], NULL, inc_plain, NULL);
    for (int i = 0; i < THREADS; i++) pthread_join(tids[i], NULL);
    printf("비원자적 카운터 결과: %ld (기대: %ld) %s\n",
           plain_counter, (long)THREADS * ITERS,
           plain_counter == THREADS * ITERS ? "✓" : "✗ race condition 발생");

    /* ── atomic_update_max 테스트 ── */
    for (int v = 1; v <= 1000; v++) atomic_update_max(v % 201);
    printf("max 업데이트 결과: %d (기대: 200)\n", atomic_load(&current_max));

    /* ── spinlock 테스트 ── */
    for (int i = 0; i < 4; i++)
        pthread_create(&tids[i], NULL, spinlock_worker, NULL);
    for (int i = 0; i < 4; i++) pthread_join(tids[i], NULL);
    printf("spinlock 보호 결과: %d (기대: %d)\n",
           atomic_load(&protected_value), 4 * 100000);

    return 0;
}
