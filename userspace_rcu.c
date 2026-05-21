// filename: userspace_rcu.c
// gcc -std=c17 -O2 -lpthread -o userspace_rcu userspace_rcu.c
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

/* ── 단순화된 사용자 공간 RCU ──────────────── */
/*
 * 이 구현은 개념 이해를 위한 단순화 버전입니다.
 * 실제 liburcu는 훨씬 정교한 quiescent state 탐지를 사용합니다.
 */

#define MAX_THREADS 16

/* reader 카운터: 각 reader가 rcu_read_lock()마다 증가,
   rcu_read_unlock()마다 감소 */
struct __attribute__((aligned(64))) ReaderCount {
    _Atomic long count;
};
static struct ReaderCount reader_counts[MAX_THREADS];
static __thread int tl_rcu_id = -1;
static _Atomic int rcu_thread_count = 0;

static void rcu_register_thread(void) {
    if (tl_rcu_id >= 0) return;
    tl_rcu_id = atomic_fetch_add(&rcu_thread_count, 1);
}

/* 읽기 섹션 진입: wait-free (단순 카운터 증가) */
static void urcu_read_lock(void) {
    atomic_fetch_add_explicit(&reader_counts[tl_rcu_id].count, 1,
                              memory_order_acquire);
}

/* 읽기 섹션 종료 */
static void urcu_read_unlock(void) {
    atomic_fetch_sub_explicit(&reader_counts[tl_rcu_id].count, 1,
                              memory_order_release);
}

/* Grace period: 모든 현재 reader가 읽기 섹션을 빠져나갈 때까지 대기 */
static void urcu_synchronize(void) {
    /* 단순 구현: 모든 reader count가 0이 될 때까지 반복 확인 */
    /* 실제 liburcu는 quiescent state 기반으로 더 효율적으로 구현 */
    int n = atomic_load(&rcu_thread_count);
    int all_clear;
    do {
        all_clear = 1;
        for (int i = 0; i < n; i++) {
            if (atomic_load_explicit(&reader_counts[i].count,
                                     memory_order_acquire) > 0) {
                all_clear = 0;
                break;
            }
        }
        /* 바쁜 대기 최소화: 실제 구현에서는 sched_yield() 또는 futex 사용 */
    } while (!all_clear);
}

/* ── RCU 보호 연결 리스트 (읽기 전용 순회 최적화) ── */
typedef struct RCUNode {
    int key;
    int value;
    _Atomic(struct RCUNode *) next;
} RCUNode;

static _Atomic(RCUNode *) rcu_list_head = NULL;

/* 읽기: lock-free, wait-free */
int rcu_list_find(int key) {
    urcu_read_lock();
    RCUNode *p = atomic_load_explicit(&rcu_list_head, memory_order_consume);
    while (p) {
        if (p->key == key) {
            int val = p->value;
            urcu_read_unlock();
            return val;
        }
        p = atomic_load_explicit(&p->next, memory_order_consume);
    }
    urcu_read_unlock();
    return -1;
}

/* 삽입: Copy-Update 패턴, grace period 불필요 (포인터 원자적 교체) */
void rcu_list_insert(int key, int value) {
    RCUNode *new_node = malloc(sizeof(RCUNode));
    new_node->key   = key;
    new_node->value = value;

    RCUNode *old_head;
    do {
        old_head = atomic_load_explicit(&rcu_list_head, memory_order_acquire);
        atomic_store_explicit(&new_node->next, old_head, memory_order_relaxed);
    } while (!atomic_compare_exchange_weak_explicit(
                 &rcu_list_head, &old_head, new_node,
                 memory_order_release, memory_order_relaxed));
}

/* 삭제: Copy-Update 패턴, grace period 후 old 노드 해제 */
int rcu_list_delete(int key) {
    /* 단순화: 헤드 노드만 삭제하는 경우를 시뮬레이션 */
    RCUNode *old_head = atomic_load_explicit(&rcu_list_head, memory_order_acquire);
    if (!old_head || old_head->key != key) return 0;

    RCUNode *new_head = atomic_load_explicit(&old_head->next, memory_order_acquire);
    if (!atomic_compare_exchange_strong_explicit(
            &rcu_list_head, &old_head, new_head,
            memory_order_release, memory_order_acquire))
        return 0;

    /* Grace period: 현재 reader들이 old_head 접근을 마칠 때까지 대기 */
    urcu_synchronize();
    free(old_head);  /* 이제 안전하게 해제 가능 */
    return 1;
}

/* ── 벤치마크: reader 8개 vs mutex 리스트 ─────────────── */
#define READERS  8
#define WRITERS  2
#define READ_OPS 100000
#define WRITE_OPS 1000

static _Atomic long read_count = 0;
static _Atomic int  stop_flag  = 0;

void *rcu_reader(void *arg) {
    (void)arg;
    rcu_register_thread();
    while (!atomic_load_explicit(&stop_flag, memory_order_acquire)) {
        int val = rcu_list_find(rand() % 100);
        (void)val;
        atomic_fetch_add_explicit(&read_count, 1, memory_order_relaxed);
    }
    return NULL;
}

void *rcu_writer(void *arg) {
    (void)arg;
    rcu_register_thread();
    for (int i = 0; i < WRITE_OPS; i++) {
        rcu_list_insert(rand() % 100, rand() % 1000);
        struct timespec ts = {0, 1000000};  /* 1ms 간격 */
        nanosleep(&ts, NULL);
    }
    return NULL;
}

int main(void) {
    /* 초기 데이터 삽입 */
    for (int i = 0; i < MAX_THREADS; i++) rcu_register_thread();

    rcu_register_thread();
    for (int i = 0; i < 50; i++) rcu_list_insert(i, i * 10);

    pthread_t readers[READERS], writers[WRITERS];

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (int i = 0; i < READERS; i++) pthread_create(&readers[i], NULL, rcu_reader, NULL);
    for (int i = 0; i < WRITERS; i++) pthread_create(&writers[i], NULL, rcu_writer, NULL);

    for (int i = 0; i < WRITERS; i++) pthread_join(writers[i], NULL);
    atomic_store_explicit(&stop_flag, 1, memory_order_release);
    for (int i = 0; i < READERS; i++) pthread_join(readers[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &t1);

    double elapsed = (t1.tv_sec - t0.tv_sec) * 1000.0 +
                     (t1.tv_nsec - t0.tv_nsec) / 1e6;

    printf("RCU 리스트 결과:\n");
    printf("  읽기 연산 수: %ld\n", atomic_load(&read_count));
    printf("  경과 시간: %.2f ms\n", elapsed);
    printf("  초당 읽기: %.0f ops/s\n",
           atomic_load(&read_count) / (elapsed / 1000.0));

    /* 특정 키 찾기 테스트 */
    int v = rcu_list_find(10);
    printf("  key=10 검색 결과: %d (기대: 100)\n", v);

    return 0;
}
