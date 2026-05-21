// filename: heap_sort.c
// gcc -std=c17 -O2 -o heap_sort heap_sort.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

static void swap(int *a, int *b) {
    int t = *a; *a = *b; *b = t;
}

/* max-heap: i보다 큰 자식이 있으면 아래로 내림 */
static void sift_down_max(int *arr, int heap_size, int i) {
    while (true) {
        int largest = i;
        int left  = 2 * i + 1;
        int right = 2 * i + 2;

        if (left  < heap_size && arr[left]  > arr[largest]) largest = left;
        if (right < heap_size && arr[right] > arr[largest]) largest = right;

        if (largest == i) break;
        swap(&arr[i], &arr[largest]);
        i = largest;
    }
}

/* O(n) buildHeap */
static void build_max_heap(int *arr, int n) {
    for (int i = n/2 - 1; i >= 0; i--)
        sift_down_max(arr, n, i);
}

void heap_sort(int *arr, int n) {
    /* 1단계: max-heap 구성 */
    build_max_heap(arr, n);

    /* 2단계: 하나씩 꺼내 정렬 */
    for (int size = n; size > 1; size--) {
        swap(&arr[0], &arr[size-1]);   /* 최댓값을 끝으로 */
        sift_down_max(arr, size-1, 0); /* 힙 속성 복원 */
    }
}

/* buildHeap 단계별 출력 */
static void print_arr(const char *label, const int *arr, int n) {
    printf("%s: [", label);
    for (int i = 0; i < n; i++) {
        printf("%d", arr[i]);
        if (i < n-1) printf(", ");
    }
    printf("]\n");
}

/* cache miss 비교 시뮬레이션 */
static long benchmark(void (*fn)(int*, int), const int *original, int n) {
    int *arr = malloc(sizeof(int) * (size_t)n);
    memcpy(arr, original, sizeof(int) * (size_t)n);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    fn(arr, n);
    clock_gettime(CLOCK_MONOTONIC, &t1);

    free(arr);
    return (t1.tv_sec - t0.tv_sec) * 1000000L +
           (t1.tv_nsec - t0.tv_nsec) / 1000;
}

/* Quick Sort (비교용) */
static void insertion_sort_small(int *arr, int left, int right) {
    for (int i = left+1; i <= right; i++) {
        int key = arr[i], j = i-1;
        while (j >= left && arr[j] > key) { arr[j+1] = arr[j]; j--; }
        arr[j+1] = key;
    }
}
static void quick_sort_helper(int *arr, int l, int r);
static int qs_partition(int *arr, int l, int r) {
    int mid = l + (r-l)/2;
    if (arr[l] > arr[mid]) swap(&arr[l], &arr[mid]);
    if (arr[l] > arr[r])   swap(&arr[l], &arr[r]);
    if (arr[mid] > arr[r]) swap(&arr[mid], &arr[r]);
    swap(&arr[mid], &arr[r-1]);
    int pivot = arr[r-1], i = l, j = r-1;
    while (1) {
        while (arr[++i] < pivot);
        while (arr[--j] > pivot);
        if (i >= j) break;
        swap(&arr[i], &arr[j]);
    }
    swap(&arr[i], &arr[r-1]);
    return i;
}
static void quick_sort_helper(int *arr, int l, int r) {
    if (r - l < 16) { insertion_sort_small(arr, l, r); return; }
    int p = qs_partition(arr, l, r);
    quick_sort_helper(arr, l, p-1);
    quick_sort_helper(arr, p+1, r);
}
void quick_sort(int *arr, int n) {
    if (n > 1) quick_sort_helper(arr, 0, n-1);
}

int main(void) {
    /* buildHeap 시각화 */
    printf("=== buildHeap 단계별 시각화 ===\n");
    int demo[] = {4, 10, 3, 5, 1, 8, 7, 2, 9, 6};
    int nd = 10;
    print_arr("원본", demo, nd);

    /* 단계별 siftDown 추적 */
    int *work = malloc(sizeof(int) * nd);
    memcpy(work, demo, sizeof(int) * nd);
    for (int i = nd/2 - 1; i >= 0; i--) {
        sift_down_max(work, nd, i);
        char label[32];
        snprintf(label, sizeof(label), "siftDown(%d) 후", i);
        print_arr(label, work, nd);
    }
    printf("→ 최대 힙 완성, root=%d\n\n", work[0]);

    /* heap sort 전체 */
    heap_sort(work, nd);
    print_arr("정렬 결과", work, nd);
    free(work);

    /* 성능 벤치마크 */
    printf("\n=== Heap Sort vs Quick Sort 벤치마크 (단위: μs) ===\n");
    const int N = 200000;
    int *arr = malloc(sizeof(int) * N);
    srand(42);
    for (int i = 0; i < N; i++) arr[i] = rand();

    printf("n=%d, 무작위 배열:\n", N);
    printf("  Heap Sort:  %ld μs\n", benchmark(heap_sort, arr, N));
    printf("  Quick Sort: %ld μs\n", benchmark(quick_sort, arr, N));

    /* 이미 정렬된 배열 (Quick Sort 유리) */
    for (int i = 0; i < N; i++) arr[i] = i;
    printf("n=%d, 정렬된 배열:\n", N);
    printf("  Heap Sort:  %ld μs\n", benchmark(heap_sort, arr, N));
    printf("  Quick Sort: %ld μs\n", benchmark(quick_sort, arr, N));

    free(arr);
    return 0;
}
