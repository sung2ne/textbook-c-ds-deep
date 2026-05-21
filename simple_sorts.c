// filename: simple_sorts.c
// gcc -std=c17 -O2 -o simple_sorts simple_sorts.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

typedef struct {
    long long comparisons;
    long long writes;
} SortStats;

static SortStats g_stats;

static void reset_stats(void) { g_stats.comparisons = g_stats.writes = 0; }

static bool cmp_and_count(int a, int b) {
    g_stats.comparisons++;
    return a > b;
}

static void write_and_count(int *dst, int val) {
    g_stats.writes++;
    *dst = val;
}

void bubble_sort_stat(int *arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        bool swapped = false;
        for (int j = 0; j < n - 1 - i; j++) {
            if (cmp_and_count(arr[j], arr[j+1])) {
                int tmp = arr[j];
                write_and_count(&arr[j],   arr[j+1]);
                write_and_count(&arr[j+1], tmp);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

void selection_sort_stat(int *arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            g_stats.comparisons++;
            if (arr[j] < arr[min_idx]) min_idx = j;
        }
        if (min_idx != i) {
            int tmp = arr[i];
            write_and_count(&arr[i],       arr[min_idx]);
            write_and_count(&arr[min_idx], tmp);
        }
    }
}

void insertion_sort_stat(int *arr, int n) {
    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && cmp_and_count(arr[j], key)) {
            write_and_count(&arr[j+1], arr[j]);
            j--;
        }
        write_and_count(&arr[j+1], key);
    }
}

static void print_arr(const int *arr, int n) {
    printf("[");
    for (int i = 0; i < n && i < 10; i++) {
        printf("%d", arr[i]);
        if (i < n - 1 && i < 9) printf(", ");
    }
    if (n > 10) printf(", ...");
    printf("]");
}

typedef void (*SortFunc)(int*, int);

static void run_test(const char *name, SortFunc fn,
                     const int *original, int n) {
    int *arr = malloc(sizeof(int) * (size_t)n);
    memcpy(arr, original, sizeof(int) * (size_t)n);

    reset_stats();
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    fn(arr, n);
    clock_gettime(CLOCK_MONOTONIC, &t1);

    long ns = (t1.tv_sec - t0.tv_sec) * 1000000000L +
              (t1.tv_nsec - t0.tv_nsec);

    printf("  %-20s 비교=%7lld  쓰기=%7lld  시간=%6ld us\n",
           name, g_stats.comparisons, g_stats.writes, ns / 1000);
    free(arr);
}

int main(void) {
    const int N = 1000;
    int *random_arr  = malloc(sizeof(int) * N);
    int *sorted_arr  = malloc(sizeof(int) * N);
    int *reverse_arr = malloc(sizeof(int) * N);

    srand(42);
    for (int i = 0; i < N; i++) {
        random_arr[i]  = rand() % 10000;
        sorted_arr[i]  = i;
        reverse_arr[i] = N - i;
    }

    printf("=== n=%d, 무작위 배열 ===\n", N);
    run_test("Bubble Sort",    bubble_sort_stat,    random_arr, N);
    run_test("Selection Sort", selection_sort_stat, random_arr, N);
    run_test("Insertion Sort", insertion_sort_stat, random_arr, N);

    printf("\n=== n=%d, 이미 정렬된 배열 ===\n", N);
    run_test("Bubble Sort",    bubble_sort_stat,    sorted_arr, N);
    run_test("Selection Sort", selection_sort_stat, sorted_arr, N);
    run_test("Insertion Sort", insertion_sort_stat, sorted_arr, N);

    printf("\n=== n=%d, 역순 정렬된 배열 ===\n", N);
    run_test("Bubble Sort",    bubble_sort_stat,    reverse_arr, N);
    run_test("Selection Sort", selection_sort_stat, reverse_arr, N);
    run_test("Insertion Sort", insertion_sort_stat, reverse_arr, N);

    free(random_arr); free(sorted_arr); free(reverse_arr);
    return 0;
}
