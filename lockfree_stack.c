// filename: lockfree_stack.c
// gcc -std=c17 -O2 -lpthread -o lockfree_stack lockfree_stack.c
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>

/* ── Treiber Lock-Free Stack ─────────────────── */
typedef struct StackNode {
    int value;
    struct StackNode *next;
} StackNode;

typedef struct {
    _Atomic(StackNode *) head;
} LFStack;

void lf_stack_init(LFStack *s) {
    atomic_store(&s->head, NULL);
}

void lf_stack_push(LFStack *s, StackNode *node) {
    StackNode *old_head;
    do {
        old_head = atomic_load_explicit(&s->head, memory_order_relaxed);
        node->next = old_head;
        /* release: node->next 쓰기가 head CAS 이전에 완료되도록 */
    } while (!atomic_compare_exchange_weak_explicit(
                 &s->head, &old_head, node,
                 memory_order_release,
                 memory_order_relaxed));
}

StackNode *lf_stack_pop(LFStack *s) {
    StackNode *old_head, *new_head;
    do {
        /* acquire: pop 성공 후 node 내용 읽기가 안전하도록 */
        old_head = atomic_load_explicit(&s->head, memory_order_acquire);
        if (!old_head) return NULL;
        new_head = old_head->next;
    } while (!atomic_compare_exchange_weak_explicit(
                 &s->head, &old_head, new_head,
                 memory_order_acquire,
                 memory_order_relaxed));
    return old_head;
}

int lf_stack_is_empty(LFStack *s) {
    return atomic_load_explicit(&s->head, memory_order_acquire) == NULL;
}

/* ── 멀티스레드 테스트 ───────────────────────── */
#define THREADS     4
#define OPS_PER_TH  100000
#define POOL_SIZE   (THREADS * OPS_PER_TH)

static LFStack g_stack;
static StackNode node_pool[POOL_SIZE];    /* 노드 풀 (ABA 완전 방지는 다음 챕터) */
static _Atomic int alloc_idx = 0;

StackNode *alloc_node(int val) {
    int idx = atomic_fetch_add_explicit(&alloc_idx, 1, memory_order_relaxed);
    node_pool[idx].value = val;
    node_pool[idx].next  = NULL;
    return &node_pool[idx];
}

static _Atomic long push_total = 0;
static _Atomic long pop_total  = 0;

void *push_worker(void *arg) {
    int id = *(int*)arg;
    for (int i = 0; i < OPS_PER_TH; i++) {
        StackNode *n = alloc_node(id * OPS_PER_TH + i);
        lf_stack_push(&g_stack, n);
        atomic_fetch_add_explicit(&push_total, 1, memory_order_relaxed);
    }
    return NULL;
}

void *pop_worker(void *arg) {
    (void)arg;
    long local_sum = 0;
    for (int i = 0; i < OPS_PER_TH; i++) {
        StackNode *n;
        while ((n = lf_stack_pop(&g_stack)) == NULL)
            ; /* 빈 경우 재시도 */
        local_sum++;
    }
    atomic_fetch_add_explicit(&pop_total, local_sum, memory_order_relaxed);
    return NULL;
}

static inline uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int main(void) {
    lf_stack_init(&g_stack);

    pthread_t push_tids[THREADS], pop_tids[THREADS];
    int ids[THREADS];
    for (int i = 0; i < THREADS; i++) ids[i] = i;

    uint64_t t0 = now_ns();
    for (int i = 0; i < THREADS; i++) {
        pthread_create(&push_tids[i], NULL, push_worker, &ids[i]);
        pthread_create(&pop_tids[i],  NULL, pop_worker,  &ids[i]);
    }
    for (int i = 0; i < THREADS; i++) {
        pthread_join(push_tids[i], NULL);
        pthread_join(pop_tids[i],  NULL);
    }
    uint64_t t1 = now_ns();

    printf("Lock-Free Stack 테스트 결과:\n");
    printf("  push 완료: %ld\n", atomic_load(&push_total));
    printf("  pop  완료: %ld\n", atomic_load(&pop_total));
    printf("  시간: %.2f ms\n", (t1-t0)/1e6);
    printf("  남은 원소: %s\n", lf_stack_is_empty(&g_stack) ? "0 (정상)" : "있음 (비정상)");
    return 0;
}
