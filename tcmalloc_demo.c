// filename: tcmalloc_demo.c
// tcmalloc 사용 예시 (gperftools 설치 필요)
// Ubuntu: sudo apt install libgoogle-perftools-dev
// gcc -std=c17 -O2 -o tcmalloc_demo tcmalloc_demo.c -ltcmalloc
// 또는 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libtcmalloc.so ./program

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1000000

static double elapsed_ms(struct timespec s, struct timespec e) {
    return (e.tv_sec - s.tv_sec) * 1e3
         + (e.tv_nsec - s.tv_nsec) / 1e6;
}

int main(void) {
    void **ptrs = malloc(N * sizeof(void*));
    struct timespec t1, t2;

    // 소형 할당 성능 측정
    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (int i = 0; i < N; i++) {
        ptrs[i] = malloc(64);
    }
    for (int i = 0; i < N; i++) {
        free(ptrs[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);

    printf("1M alloc+free of 64B: %.1f ms\n", elapsed_ms(t1, t2));

    free(ptrs);
    return 0;
}
