// filename: arena_demo.c
// gcc -std=c17 -O2 -pthread -o arena_demo arena_demo.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <malloc.h>

static void *thread_func(void *arg) {
    // 각 스레드에서 malloc을 호출하면 별도 arena를 사용합니다.
    void *p = malloc(64);
    printf("Thread %lu: allocated at %p\n", pthread_self(), p);

    // malloc_stats()로 arena 개수를 출력할 수 있습니다.
    // (리눅스 glibc 전용)
    free(p);
    return NULL;
}

int main(void) {
    pthread_t threads[4];
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, thread_func, NULL);
    }
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    // main 스레드에서 arena 정보 출력 (glibc 확장)
    malloc_stats();
    return 0;
}
