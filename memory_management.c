// filename: memory_management.c
// gcc -std=c17 -O2 -o memory_management memory_management.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 동적 배열 구조체 */
typedef struct {
    int*   data;
    size_t size;       /* 현재 원소 수 */
    size_t capacity;   /* 할당된 공간 */
} IntArray;

/* 초기화 */
IntArray ia_create(size_t initial_cap) {
    IntArray ia;
    ia.data = malloc(sizeof(int) * initial_cap);
    if (ia.data == NULL && initial_cap > 0) {
        fprintf(stderr, "ia_create: malloc 실패\n");
        exit(EXIT_FAILURE);
    }
    ia.size = 0;
    ia.capacity = initial_cap;
    return ia;
}

/* 원소 추가 — 필요 시 2배 확장 */
void ia_push(IntArray* ia, int value) {
    if (ia->size == ia->capacity) {
        size_t new_cap = (ia->capacity == 0) ? 1 : ia->capacity * 2;
        int* tmp = realloc(ia->data, sizeof(int) * new_cap);
        if (tmp == NULL) {
            fprintf(stderr, "ia_push: realloc 실패\n");
            exit(EXIT_FAILURE);
        }
        ia->data = tmp;
        ia->capacity = new_cap;
    }
    ia->data[ia->size++] = value;
}

/* 마지막 원소 꺼내기 */
int ia_pop(IntArray* ia) {
    if (ia->size == 0) {
        fprintf(stderr, "ia_pop: 빈 배열\n");
        exit(EXIT_FAILURE);
    }
    return ia->data[--ia->size];
}

/* 해제 */
void ia_destroy(IntArray* ia) {
    free(ia->data);
    ia->data = NULL;
    ia->size = 0;
    ia->capacity = 0;
}

int main(void) {
    IntArray arr = ia_create(2);

    /* 원소 추가 — capacity를 초과하면 자동 확장 */
    for (int i = 0; i < 10; i++) {
        ia_push(&arr, i * 10);
        printf("push %d → size=%zu, capacity=%zu\n",
               i * 10, arr.size, arr.capacity);
    }

    /* 출력 */
    printf("\n배열 내용: ");
    for (size_t i = 0; i < arr.size; i++) {
        printf("%d ", arr.data[i]);
    }
    printf("\n");

    /* 원소 꺼내기 */
    printf("pop: %d\n", ia_pop(&arr));
    printf("pop: %d\n", ia_pop(&arr));

    /* 해제 */
    ia_destroy(&arr);
    printf("메모리 해제 완료\n");

    /* calloc 예제 */
    int* zeros = calloc(5, sizeof(int));
    if (zeros == NULL) { exit(EXIT_FAILURE); }
    printf("\ncalloc 초기값: ");
    for (int i = 0; i < 5; i++) printf("%d ", zeros[i]);
    printf("\n");
    free(zeros);
    zeros = NULL;

    return 0;
}
