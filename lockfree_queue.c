// filename: lockfree_queue.c
// gcc -std=c17 -O2 -lpthread -o lockfree_queue lockfree_queue.c
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdalign.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>

/* ── Michael-Scott Lock-Free Queue ──────────── */
typedef struct QNode {
    int value;
    _Atomic(struct QNode *) next;
} QNode;

typedef struct {
    alignas(64) _Atomic(QNode *) head;   /* consumer 측 */
    alignas(64) _Atomic(QNode *) tail;   /* producer 측 */
} LFQueue;

void lf_queue_init(LFQueue *q) {
    /* dummy sentinel 노드 */
    QNode *dummy = calloc(1, sizeof(QNode));
    dummy->value = -1;
    atomic_store(&dummy->next, NULL);
    atomic_store(&q->head, dummy);
    atomic_store(&q->tail, dummy);
}

void lf_queue_enqueue(LFQueue *q, QNode *node) {
    atomic_store_explicit(&node->next, NULL, memory_order_relaxed);

    QNode *old_tail, *null_ptr;
    while (1) {
        old_tail = atomic_load_explicit(&q->tail, memory_order_acquire);
        QNode *next = atomic_load_explicit(&old_tail->next, memory_order_acquire);

        /* tail이 아직 실제 꼬리인지 확인 */
        if (old_tail != atomic_load_explicit(&q->tail, memory_order_acquire))
            continue;

        if (next == NULL) {
            /* tail->next가 NULL이면 새 노드를 연결 */
            null_ptr = NULL;
            if (atomic_compare_exchange_weak_explicit(
                    &old_tail->next, &null_ptr, node,
                    memory_order_release,
                    memory_order_relaxed)) {
                break;  /* 성공 */
            }
        } else {
            /* 다른 스레드가 이미 노드를 추가했지만 tail을 아직 이동 못 함 */
            /* tail 전진을 도와준다 */
            atomic_compare_exchange_weak_explicit(
                &q->tail, &old_tail, next,
                memory_order_release,
                memory_order_relaxed);
        }
    }
    /* tail 전진 (실패해도 괜찮음: 다른 스레드가 처리) */
    atomic_compare_exchange_weak_explicit(
        &q->tail, &old_tail, node,
        memory_order_release,
        memory_order_relaxed);
}

/* sentinel(더미)를 반환. 실제 값은 sentinel->next에서 꺼냄 */
QNode *lf_queue_dequeue(LFQueue *q) {
    QNode *old_head, *old_tail, *next;
    while (1) {
        old_head = atomic_load_explicit(&q->head, memory_order_acquire);
        old_tail = atomic_load_explicit(&q->tail, memory_order_acquire);
        next     = atomic_load_explicit(&old_head->next, memory_order_acquire);

        if (old_head != atomic_load_explicit(&q->head, memory_order_acquire))
            continue;

        if (old_head == old_tail) {
            /* 큐가 비어있거나 tail이 뒤처진 경우 */
            if (next == NULL) return NULL;   /* 실제로 빈 큐 */
            /* tail 전진 도와주기 */
            atomic_compare_exchange_weak_explicit(
                &q->tail, &old_tail, next,
                memory_order_release, memory_order_relaxed);
        } else {
            /* next의 값을 읽고 head를 next로 이동 */
            int val = next->value;
            if (atomic_compare_exchange_weak_explicit(
                    &q->head, &old_head, next,
                    memory_order_release,
                    memory_order_relaxed)) {
                /* old_head(sentinel)가 새 sentinel이 됨 */
                /* next가 데이터를 가지고 있음 */
                old_head->value = val;   /* 반환용 (재사용) */
                return old_head;
            }
        }
    }
}

int lf_queue_is_empty(LFQueue *q) {
    QNode *head = atomic_load_explicit(&q->head, memory_order_acquire);
    QNode *next = atomic_load_explicit(&head->next, memory_order_acquire);
    return next == NULL;
}

#define THREADS    4
#define OPS        50000

static LFQueue g_queue;
static _Atomic long enq_count = 0, deq_count = 0;

void *enqueue_worker(void *arg) {
    int id = *(int*)arg;
    for (int i = 0; i < OPS; i++) {
        QNode *n = malloc(sizeof(QNode));
        n->value = id * OPS + i;
        lf_queue_enqueue(&g_queue, n);
        atomic_fetch_add_explicit(&enq_count, 1, memory_order_relaxed);
    }
    return NULL;
}

void *dequeue_worker(void *arg) {
    (void)arg;
    for (int i = 0; i < OPS; i++) {
        QNode *n;
        while ((n = lf_queue_dequeue(&g_queue)) == NULL)
            ;
        free(n);
        atomic_fetch_add_explicit(&deq_count, 1, memory_order_relaxed);
    }
    return NULL;
}

int main(void) {
    lf_queue_init(&g_queue);

    pthread_t enq_tids[THREADS], deq_tids[THREADS];
    int ids[THREADS];
    for (int i = 0; i < THREADS; i++) ids[i] = i;

    for (int i = 0; i < THREADS; i++) {
        pthread_create(&enq_tids[i], NULL, enqueue_worker, &ids[i]);
        pthread_create(&deq_tids[i], NULL, dequeue_worker, &ids[i]);
    }
    for (int i = 0; i < THREADS; i++) {
        pthread_join(enq_tids[i], NULL);
        pthread_join(deq_tids[i], NULL);
    }

    printf("Lock-Free Queue 결과:\n");
    printf("  enqueue: %ld\n", atomic_load(&enq_count));
    printf("  dequeue: %ld\n", atomic_load(&deq_count));
    printf("  비어있음: %s\n", lf_queue_is_empty(&g_queue) ? "예 (정상)" : "아니오");
    return 0;
}
