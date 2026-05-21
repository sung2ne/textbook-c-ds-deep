// filename: static_array.c
// gcc -std=c17 -O2 -o static_array static_array.c

#include <stdio.h>
#include <string.h>

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

/* 배열을 함수에 전달: 포인터 + 크기 */
int sum(const int* arr, int n) {
    int total = 0;
    for (int i = 0; i < n; i++) total += arr[i];
    return total;
}

/* 최솟값 인덱스 */
int min_index(const int* arr, int n) {
    int idx = 0;
    for (int i = 1; i < n; i++) {
        if (arr[i] < arr[idx]) idx = i;
    }
    return idx;
}

/* 배열 역순 (in-place) */
void reverse(int* arr, int n) {
    for (int lo = 0, hi = n - 1; lo < hi; lo++, hi--) {
        int tmp = arr[lo];
        arr[lo] = arr[hi];
        arr[hi] = tmp;
    }
}

/* 이진 탐색 (정렬된 배열) */
int binary_search(const int* arr, int n, int target) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;   /* 오버플로우 방지 */
        if (arr[mid] == target) return mid;
        if (arr[mid] < target) lo = mid + 1;
        else                   hi = mid - 1;
    }
    return -1;   /* 없음 */
}

/* 2D 배열: 행렬 전치 (소규모) */
#define N 3
void transpose(int mat[N][N]) {
    for (int r = 0; r < N; r++) {
        for (int c = r + 1; c < N; c++) {
            int tmp = mat[r][c];
            mat[r][c] = mat[c][r];
            mat[c][r] = tmp;
        }
    }
}

int main(void) {
    /* 기본 배열 */
    int arr[] = {5, 3, 8, 1, 9, 2, 7, 4, 6};
    int n = (int)ARRAY_LEN(arr);

    printf("배열: ");
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");
    printf("합: %d\n", sum(arr, n));
    printf("최솟값 인덱스: %d (값: %d)\n", min_index(arr, n), arr[min_index(arr, n)]);

    /* 역순 */
    reverse(arr, n);
    printf("역순: ");
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");

    /* 이진 탐색 (정렬된 배열 필요) */
    int sorted[] = {1, 3, 5, 7, 9, 11, 13, 15};
    int sn = (int)ARRAY_LEN(sorted);
    printf("\n이진 탐색:\n");
    int targets[] = {7, 4, 15};
    for (int i = 0; i < 3; i++) {
        int idx = binary_search(sorted, sn, targets[i]);
        if (idx >= 0) printf("  %d → 인덱스 %d\n", targets[i], idx);
        else          printf("  %d → 없음\n", targets[i]);
    }

    /* 2D 배열 전치 */
    int mat[N][N] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    printf("\n전치 전:\n");
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) printf("%3d", mat[r][c]);
        printf("\n");
    }
    transpose(mat);
    printf("전치 후:\n");
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) printf("%3d", mat[r][c]);
        printf("\n");
    }

    return 0;
}
