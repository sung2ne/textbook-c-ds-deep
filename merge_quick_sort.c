// filename: merge_quick_sort.c
// gcc -std=c17 -O2 -o merge_quick_sort merge_quick_sort.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

/* ---- Merge Sort ---- */
static void merge(int *arr, int *tmp, int left, int mid, int right) {
    /* tmp에 복사 */
    memcpy(tmp + left, arr + left, sizeof(int) * (size_t)(right - left + 1));

    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        if (tmp[i] <= tmp[j]) arr[k++] = tmp[i++];  /* <= 로 안정성 보장 */
        else                  arr[k++] = tmp[j++];
    }
    while (i <= mid)  arr[k++] = tmp[i++];
    while (j <= right) arr[k++] = tmp[j++];
}

static void merge_sort_helper(int *arr, int *tmp, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;  /* 오버플로우 방지 */
    merge_sort_helper(arr, tmp, left,    mid);
    merge_sort_helper(arr, tmp, mid + 1, right);
    merge(arr, tmp, left, mid, right);
}

void merge_sort(int *arr, int n) {
    if (n <= 1) return;
    int *tmp = malloc(sizeof(int) * (size_t)n);
    if (!tmp) { perror("malloc"); return; }
    merge_sort_helper(arr, tmp, 0, n - 1);
    free(tmp);
}

/* ---- Quick Sort ---- */
/* 3-median pivot 선택 */
static int median_of_three(int *arr, int left, int right) {
    int mid = left + (right - left) / 2;
    /* 세 값을 정렬하여 mid에 중앙값 배치 */
    if (arr[left] > arr[mid]) { int t = arr[left]; arr[left] = arr[mid]; arr[mid] = t; }
    if (arr[left] > arr[right]){ int t = arr[left]; arr[left] = arr[right]; arr[right] = t; }
    if (arr[mid]  > arr[right]){ int t = arr[mid];  arr[mid]  = arr[right]; arr[right] = t; }
    /* arr[left] <= arr[mid] <= arr[right] */
    /* pivot(mid)을 right-1로 이동 */
    int t = arr[mid]; arr[mid] = arr[right-1]; arr[right-1] = t;
    return arr[right-1];
}

/* Hoare-style partition */
static int partition(int *arr, int left, int right) {
    if (right - left < 2) {
        if (arr[left] > arr[right]) {
            int t = arr[left]; arr[left] = arr[right]; arr[right] = t;
        }
        return left;
    }
    int pivot = median_of_three(arr, left, right);
    int i = left, j = right - 1;
    while (true) {
        while (arr[++i] < pivot);
        while (arr[--j] > pivot);
        if (i >= j) break;
        int t = arr[i]; arr[i] = arr[j]; arr[j] = t;
    }
    /* pivot을 최종 위치에 */
    int t = arr[i]; arr[i] = arr[right-1]; arr[right-1] = t;
    return i;
}

/* n이 작으면 Insertion Sort로 전환 */
#define INSERTION_THRESHOLD 16

static void insertion_sort_range(int *arr, int left, int right) {
    for (int i = left + 1; i <= right; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= left && arr[j] > key) { arr[j+1] = arr[j]; j--; }
        arr[j+1] = key;
    }
}

static void quick_sort_helper(int *arr, int left, int right) {
    if (right - left < INSERTION_THRESHOLD) {
        insertion_sort_range(arr, left, right);
        return;
    }
    int pivot_idx = partition(arr, left, right);
    quick_sort_helper(arr, left,         pivot_idx - 1);
    quick_sort_helper(arr, pivot_idx + 1, right);
}

void quick_sort(int *arr, int n) {
    if (n <= 1) return;
    quick_sort_helper(arr, 0, n - 1);
}

/* ---- 성능 비교 ---- */
static void print_first10(const int *arr, int n) {
    printf("[");
    for (int i = 0; i < n && i < 10; i++) {
        printf("%d", arr[i]);
        if (i < n - 1 && i < 9) printf(", ");
    }
    if (n > 10) printf(", ...");
    printf("]\n");
}

static long benchmark(void (*fn)(int*, int), int *arr, int n) {
    int *copy = malloc(sizeof(int) * (size_t)n);
    memcpy(copy, arr, sizeof(int) * (size_t)n);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    fn(copy, n);
    clock_gettime(CLOCK_MONOTONIC, &t1);

    free(copy);
    return (t1.tv_sec - t0.tv_sec) * 1000000L +
           (t1.tv_nsec - t0.tv_nsec) / 1000;
}

int main(void) {
    /* 정확성 검증 */
    printf("=== 정확성 검증 ===\n");
    int test[] = {5, 3, 8, 1, 9, 2, 7, 4, 6};
    int n_test = sizeof(test) / sizeof(test[0]);

    int a[9], b[9];
    memcpy(a, test, sizeof(test));
    memcpy(b, test, sizeof(test));

    merge_sort(a, n_test);
    quick_sort(b, n_test);

    printf("원본:         "); print_first10(test, n_test);
    printf("Merge Sort:   "); print_first10(a, n_test);
    printf("Quick Sort:   "); print_first10(b, n_test);

    /* 성능 비교 */
    const int N = 100000;
    int *random_arr  = malloc(sizeof(int) * N);
    int *sorted_arr  = malloc(sizeof(int) * N);
    int *reverse_arr = malloc(sizeof(int) * N);

    srand(42);
    for (int i = 0; i < N; i++) {
        random_arr[i]  = rand();
        sorted_arr[i]  = i;
        reverse_arr[i] = N - i;
    }

    printf("\n=== n=%d 성능 비교 (단위: μs) ===\n", N);
    printf("%-12s  %8s  %8s  %8s\n",
           "알고리즘", "무작위", "정렬됨", "역순");
    printf("%-12s  %8ld  %8ld  %8ld\n", "Merge Sort",
           benchmark(merge_sort, random_arr,  N),
           benchmark(merge_sort, sorted_arr,  N),
           benchmark(merge_sort, reverse_arr, N));
    printf("%-12s  %8ld  %8ld  %8ld\n", "Quick Sort",
           benchmark(quick_sort, random_arr,  N),
           benchmark(quick_sort, sorted_arr,  N),
           benchmark(quick_sort, reverse_arr, N));

    free(random_arr); free(sorted_arr); free(reverse_arr);
    return 0;
}
