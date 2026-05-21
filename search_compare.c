// filename: search_compare.c
// gcc -std=c17 -O2 -o search_compare search_compare.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1000000   /* 백만 개 */

/* ─── 선형 탐색 O(n) ─── */
static int linear_search(const int *arr, int n, int target) {
    for (int i = 0; i < n; i++) {
        if (arr[i] == target) return i;
    }
    return -1;
}

/* ─── 이진 탐색 O(log n) — 정렬된 배열에서만 가능 ─── */
static int binary_search(const int *arr, int n, int target) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;   /* (lo + hi) / 2는 오버플로우 위험 */
        if      (arr[mid] == target) return mid;
        else if (arr[mid] <  target) lo = mid + 1;
        else                          hi = mid - 1;
    }
    return -1;
}

/* qsort 비교 함수 */
static int cmp_int(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

static long long now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

int main(void) {
    int *arr = malloc(sizeof(int) * N);
    if (!arr) { perror("malloc"); return 1; }

    /* 배열 초기화 (0 ~ N-1, 랜덤 섞기) */
    for (int i = 0; i < N; i++) arr[i] = i;
    /* Fisher-Yates 셔플 */
    srand(42);
    for (int i = N - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
    }

    int target = N - 1;   /* 배열에서 찾을 값 */

    /* 선형 탐색 측정 */
    long long t0 = now_ns();
    int idx1 = linear_search(arr, N, target);
    long long t1 = now_ns();
    printf("linear_search:  idx=%d, %8.3f ms\n", idx1, (t1 - t0) / 1e6);

    /* 정렬 후 이진 탐색 측정 */
    qsort(arr, N, sizeof(int), cmp_int);

    t0 = now_ns();
    int idx2 = binary_search(arr, N, target);
    t1 = now_ns();
    printf("binary_search:  idx=%d, %8.3f ms\n", idx2, (t1 - t0) / 1e6);

    free(arr);
    return 0;
}
