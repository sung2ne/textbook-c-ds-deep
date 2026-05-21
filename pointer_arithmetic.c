// filename: pointer_arithmetic.c
// gcc -std=c17 -O2 -o pointer_arithmetic pointer_arithmetic.c

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define ROWS 3
#define COLS 4

/* 2D 배열을 포인터로 받는 함수 */
void fill_matrix(int (*mat)[COLS], int rows) {
    int val = 1;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < COLS; c++) {
            mat[r][c] = val++;
        }
    }
}

void print_matrix_2d(int (*mat)[COLS], int rows) {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < COLS; c++) {
            printf("%3d ", mat[r][c]);
        }
        printf("\n");
    }
}

/* jagged array 생성 */
int** create_jagged(int rows, const int* col_sizes) {
    int** m = malloc(sizeof(int*) * (size_t)rows);
    if (!m) return NULL;
    for (int r = 0; r < rows; r++) {
        m[r] = malloc(sizeof(int) * (size_t)col_sizes[r]);
        if (!m[r]) {
            for (int i = 0; i < r; i++) free(m[i]);
            free(m);
            return NULL;
        }
        for (int c = 0; c < col_sizes[r]; c++) {
            m[r][c] = r * 10 + c;
        }
    }
    return m;
}

void destroy_jagged(int** m, int rows) {
    for (int r = 0; r < rows; r++) free(m[r]);
    free(m);
}

int main(void) {
    /* 포인터 산술 기초 */
    int arr[5] = {10, 20, 30, 40, 50};
    int* ptr = arr;

    printf("포인터 산술:\n");
    for (int i = 0; i < 5; i++) {
        printf("  arr[%d] = %d, *(arr+%d) = %d, 같은가: %s\n",
               i, arr[i], i, *(arr + i),
               (arr[i] == *(arr + i)) ? "YES" : "NO");
    }

    /* 포인터 이동 */
    printf("\n포인터 순회:\n  ");
    while (ptr < arr + 5) {
        printf("%d ", *ptr++);
    }
    printf("\n");

    /* 포인터 뺄셈 */
    ptr = arr + 3;
    ptrdiff_t diff = ptr - arr;
    printf("\narr+3 - arr = %td\n", diff);

    /* 2D 배열 */
    int matrix[ROWS][COLS];
    fill_matrix(matrix, ROWS);
    printf("\n2D 배열 (스택):\n");
    print_matrix_2d(matrix, ROWS);

    /* 메모리 연속성 확인 */
    printf("\n2D 배열 메모리 연속성 (1D로 접근):\n  ");
    int* flat = &matrix[0][0];
    for (int i = 0; i < ROWS * COLS; i++) {
        printf("%d ", flat[i]);
    }
    printf("\n");

    /* Jagged array */
    int col_sizes[] = {2, 4, 3};
    int** jagged = create_jagged(ROWS, col_sizes);
    if (jagged) {
        printf("\nJagged array:\n");
        for (int r = 0; r < ROWS; r++) {
            printf("  row[%d]: ", r);
            for (int c = 0; c < col_sizes[r]; c++) {
                printf("%d ", jagged[r][c]);
            }
            printf("\n");
        }
        destroy_jagged(jagged, ROWS);
    }

    return 0;
}
