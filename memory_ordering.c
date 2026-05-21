// filename: memory_ordering.c
// gcc -std=c17 -O2 -lpthread -o memory_ordering memory_ordering.c
#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

/* ── Producer-Consumer 패턴 ──────────────────── */
#define BUF_SIZE 256

static char    data_buffer[BUF_SIZE];
static _Atomic int data_ready = 0;   /* 0=미준비, 1=준비됨 */

void *producer(void *arg) {
    (void)arg;
    /* 1. 데이터 준비 */
    snprintf(data_buffer, BUF_SIZE, "Hello from producer! Answer = %d", 42);

    /* 2. release: 위의 모든 쓰기(data_buffer 채우기)가 완료된 후에
       data_ready 쓰기가 consumer에게 보임 */
    atomic_store_explicit(&data_ready, 1, memory_order_release);
    printf("[Producer] 데이터 준비 완료, data_ready = 1\n");
    return NULL;
}

void *consumer(void *arg) {
    (void)arg;
    /* 1. acquire: data_ready 읽기 성공 후에야
       data_buffer 읽기를 시작 (재배치 방지) */
    while (atomic_load_explicit(&data_ready, memory_order_acquire) == 0)
        ; /* busy-wait */

    /* 2. 여기서 data_buffer는 반드시 완전히 채워져 있음이 보장됨 */
    printf("[Consumer] 받은 데이터: %s\n", data_buffer);
    return NULL;
}

/* ── Double-Checked Locking 패턴 ─────────────── */
/* (단순화 버전, 실제로는 pthread_once 사용 권장) */

typedef struct { int initialized; int value; } Singleton;
static _Atomic(Singleton *) instance_ptr = NULL;

Singleton *get_instance(void) {
    Singleton *p = atomic_load_explicit(&instance_ptr, memory_order_acquire);
    if (p == NULL) {
        p = malloc(sizeof(Singleton));
        p->initialized = 1;
        p->value = 42;

        Singleton *expected = NULL;
        if (!atomic_compare_exchange_strong_explicit(
                &instance_ptr, &expected, p,
                memory_order_release,
                memory_order_acquire)) {
            /* 다른 스레드가 먼저 초기화함 */
            free(p);
            p = expected;  /* expected에 현재 값이 들어옴 */
        }
    }
    return p;
}

int main(void) {
    printf("=== Producer-Consumer ===\n");
    pthread_t prod, cons;
    pthread_create(&cons, NULL, consumer, NULL);   /* consumer 먼저 시작 */
    pthread_create(&prod, NULL, producer, NULL);
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    printf("\n=== Singleton ===\n");
    Singleton *s = get_instance();
    printf("value = %d, initialized = %d\n", s->value, s->initialized);
    free(s);
    return 0;
}
