// filename: tsan_demo.c
// gcc -fsanitize=thread -g -O1 -pthread -o tsan_demo tsan_demo.c

#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

/* ───────── 레이스 있는 코드 ───────── */
long counter_racy = 0;

void *increment_racy(void *arg) {
    (void)arg;
    for (int i = 0; i < 100000; i++)
        counter_racy++;  /* 동기화 없음 → 데이터 레이스! */
    return NULL;
}

/* ───────── 뮤텍스로 수정 ───────── */
long counter_safe = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *increment_safe(void *arg) {
    (void)arg;
    for (int i = 0; i < 100000; i++) {
        pthread_mutex_lock(&lock);
        counter_safe++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

/* ───────── _Atomic으로 수정 ───────── */
_Atomic long counter_atomic = 0;

void *increment_atomic(void *arg) {
    (void)arg;
    for (int i = 0; i < 100000; i++)
        counter_atomic++;  /* atomic → 레이스 없음 */
    return NULL;
}

int main(void) {
    pthread_t t1, t2;

    /* 레이스 있는 버전 */
    printf("=== 레이스 있는 버전 ===\n");
    pthread_create(&t1, NULL, increment_racy, NULL);
    pthread_create(&t2, NULL, increment_racy, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("counter_racy = %ld (예상: 200000, 실제 불일치)\n", counter_racy);

    /* 뮤텍스 버전 */
    printf("\n=== 뮤텍스 버전 ===\n");
    pthread_create(&t1, NULL, increment_safe, NULL);
    pthread_create(&t2, NULL, increment_safe, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("counter_safe = %ld (정확히 200000)\n", counter_safe);

    /* atomic 버전 */
    printf("\n=== _Atomic 버전 ===\n");
    pthread_create(&t1, NULL, increment_atomic, NULL);
    pthread_create(&t2, NULL, increment_atomic, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("counter_atomic = %ld (정확히 200000)\n", (long)counter_atomic);

    return 0;
}

/*
=== TSan이 보고하는 레이스 ===

WARNING: ThreadSanitizer: data race (pid=12345)
  Write of size 8 at 0x000001234567 by thread T2:
    #0 increment_racy tsan_demo.c:13
    #1 <null>

  Previous write of size 8 at 0x000001234567 by thread T1:
    #0 increment_racy tsan_demo.c:13
    #1 <null>

  Location is global 'counter_racy' at 0x000001234567 (tsan_demo.c:7)
*/
