// filename: hazard_pointer.c
// gcc -std=c17 -O2 -lpthread -o hazard_pointer hazard_pointer.c
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>

#define MAX_THREADS   16
#define HP_PER_THREAD 2     /* 스레드당 최대 위험 포인터 수 */
#define RETIRE_LIMIT  64    /* retired list가 이 크기 이상이면 scan */

/* ── Hazard Pointer 전역 테이블 ─────────────── */
typedef struct {
    _Atomic(void *) ptrs[HP_PER_THREAD];
} HazardRecord;

static HazardRecord hp_table[MAX_THREADS];   /* 전역 HP 테이블 */
static _Atomic int  hp_thread_count = 0;

/* 스레드 로컬 ID */
static __thread int tl_thread_id = -1;

static void hp_thread_register(void) {
    if (tl_thread_id >= 0) return;
    tl_thread_id = atomic_fetch_add(&hp_thread_count, 1);
    for (int i = 0; i < HP_PER_THREAD; i++)
        atomic_store(&hp_table[tl_thread_id].ptrs[i], NULL);
}

/* HP 등록: 이 포인터는 내가 접근 중이니 free하지 말 것 */
static void hp_protect(int slot, void *ptr) {
    atomic_store_explicit(&hp_table[tl_thread_id].ptrs[slot], ptr,
                          memory_order_seq_cst);
}

/* HP 해제: 이 포인터를 더 이상 쓰지 않음 */
static void hp_release(int slot) {
    atomic_store_explicit(&hp_table[tl_thread_id].ptrs[slot], NULL,
                          memory_order_release);
}

/* 위험 포인터로 보호되어 있는지 검사 */
static int hp_is_protected(void *ptr) {
    int n = atomic_load(&hp_thread_count);
    for (int t = 0; t < n; t++) {
        for (int s = 0; s < HP_PER_THREAD; s++) {
            if (atomic_load_explicit(&hp_table[t].ptrs[s],
                                     memory_order_acquire) == ptr)
                return 1;
        }
    }
    return 0;
}

/* ── Retired List (스레드 로컬) ─────────────── */
static __thread void *retired[RETIRE_LIMIT];
static __thread int   retired_count = 0;

/* 노드 해제를 요청: 지금 당장 free하지 않고 retired list에 등록 */
static void hp_retire(void *ptr) {
    retired[retired_count++] = ptr;
    if (retired_count < RETIRE_LIMIT) return;

    /* retired list가 가득 찼으면 scan: 보호되지 않은 것만 free */
    int new_count = 0;
    for (int i = 0; i < retired_count; i++) {
        if (hp_is_protected(retired[i])) {
            retired[new_count++] = retired[i];  /* 아직 보호됨: 유지 */
        } else {
            free(retired[i]);  /* 안전하게 해제 */
        }
    }
    retired_count = new_count;
}

/* ── HP를 적용한 Lock-Free Stack ─────────────── */
typedef struct SNode {
    int value;
    _Atomic(struct SNode *) next;
} SNode;

typedef struct {
    _Atomic(SNode *) head;
} HPStack;

void hps_init(HPStack *s) { atomic_store(&s->head, NULL); }

void hps_push(HPStack *s, SNode *node) {
    hp_thread_register();
    SNode *old_head;
    do {
        old_head = atomic_load_explicit(&s->head, memory_order_relaxed);
        atomic_store(&node->next, old_head);
    } while (!atomic_compare_exchange_weak_explicit(
                 &s->head, &old_head, node,
                 memory_order_release, memory_order_relaxed));
}

SNode *hps_pop(HPStack *s) {
    hp_thread_register();
    SNode *old_head;
    while (1) {
        /* 1단계: head를 HP 슬롯 0에 등록 */
        old_head = atomic_load_explicit(&s->head, memory_order_acquire);
        if (!old_head) return NULL;

        hp_protect(0, old_head);  /* "이 노드를 접근 중" 선언 */

        /* 2단계: head가 아직도 같은지 재확인 (등록 사이에 바뀔 수 있음) */
        if (atomic_load_explicit(&s->head, memory_order_acquire) != old_head)
            continue;   /* 바뀌었으면 재시도 */

        /* 3단계: CAS로 head를 next로 교체 */
        SNode *next = atomic_load_explicit(&old_head->next, memory_order_acquire);
        if (atomic_compare_exchange_weak_explicit(
                &s->head, &old_head, next,
                memory_order_acquire, memory_order_relaxed)) {
            hp_release(0);  /* HP 해제 */
            return old_head;  /* caller가 사용 후 hp_retire() 호출 */
        }
    }
}

/* ── 테스트 ──────────────────────────────────── */
#define THREADS 4
#define OPS     10000

static HPStack g_hps;
static _Atomic long pushed = 0, popped = 0;

void *hps_producer(void *arg) {
    (void)arg;
    hp_thread_register();
    for (int i = 0; i < OPS; i++) {
        SNode *n = malloc(sizeof(SNode));
        n->value = i;
        hps_push(&g_hps, n);
        atomic_fetch_add_explicit(&pushed, 1, memory_order_relaxed);
    }
    return NULL;
}

void *hps_consumer(void *arg) {
    (void)arg;
    hp_thread_register();
    for (int i = 0; i < OPS; i++) {
        SNode *n;
        while ((n = hps_pop(&g_hps)) == NULL) ;
        hp_retire(n);  /* 즉시 free하지 않고 retired list에 등록 */
        atomic_fetch_add_explicit(&popped, 1, memory_order_relaxed);
    }
    return NULL;
}

int main(void) {
    hps_init(&g_hps);
    pthread_t prod[THREADS], cons[THREADS];

    for (int i = 0; i < THREADS; i++) {
        pthread_create(&prod[i], NULL, hps_producer, NULL);
        pthread_create(&cons[i], NULL, hps_consumer, NULL);
    }
    for (int i = 0; i < THREADS; i++) {
        pthread_join(prod[i], NULL);
        pthread_join(cons[i], NULL);
    }

    printf("Hazard Pointer Stack:\n");
    printf("  pushed: %ld, popped: %ld\n",
           atomic_load(&pushed), atomic_load(&popped));
    printf("  비어있음: %s\n",
           atomic_load(&g_hps.head) == NULL ? "예 (정상)" : "아니오");
    return 0;
}
