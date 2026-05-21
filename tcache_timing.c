// filename: tcache_timing.c
// gcc -std=c17 -O2 -o tcache_timing tcache_timing.c

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ITER 1000000

static double elapsed_ns(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1e9
         + (end.tv_nsec - start.tv_nsec);
}

int main(void) {
    struct timespec t1, t2;

    // tcache warmup: 한 번 할당/해제하면 tcache에 청크가 들어갑니다.
    void *warmup = malloc(64);
    free(warmup);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (int i = 0; i < ITER; i++) {
        void *p = malloc(64);   // tcache에서 꺼냄 (lock 없음)
        free(p);                // tcache에 반환
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);

    double ns = elapsed_ns(t1, t2) / ITER;
    printf("tcache hit avg: %.1f ns per malloc+free\n", ns);
    return 0;
}
