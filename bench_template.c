// filename: bench_template.c
// gcc -std=c17 -O2 -o bench_template bench_template.c

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* 나노초 단위 현재 시간 */
static long long now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

/* 측정할 함수 — 예시: 배열 합계 */
static long long sum_array(const int *arr, int n) {
    long long s = 0;
    for (int i = 0; i < n; i++) s += arr[i];
    return s;
}

#define N       1000000
#define ROUNDS  10

int main(void) {
    int *arr = malloc(sizeof(int) * N);
    for (int i = 0; i < N; i++) arr[i] = i;

    /* Warm-up: 캐시와 분기 예측기를 예열 */
    volatile long long warmup = sum_array(arr, N);
    (void)warmup;

    long long total = 0;
    for (int r = 0; r < ROUNDS; r++) {
        long long t0 = now_ns();
        volatile long long result = sum_array(arr, N);
        long long t1 = now_ns();
        (void)result;
        total += (t1 - t0);
        printf("Round %2d: %6.3f ms\n", r + 1, (t1 - t0) / 1e6);
    }
    printf("Average:  %6.3f ms\n", total / (double)ROUNDS / 1e6);

    free(arr);
    return 0;
}
