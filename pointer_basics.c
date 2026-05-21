// filename: pointer_basics.c
// gcc -std=c17 -O2 -o pointer_basics pointer_basics.c

#include <stdio.h>
#include <stdlib.h>

/* 포인터로 값 교환 */
void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

/* 포인터를 수정하는 함수 */
void make_array(int** out, int size) {
    *out = malloc(sizeof(int) * (size_t)size);
    if (*out == NULL) {
        fprintf(stderr, "malloc 실패\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < size; i++) {
        (*out)[i] = i * 10;
    }
}

/* void* 제네릭 출력 */
void print_int(const void* data) {
    printf("%d", *(const int*)data);
}

int main(void) {
    /* 기본 포인터 */
    int x = 42;
    int* ptr = &x;
    printf("x의 주소: %p\n", (void*)ptr);
    printf("ptr이 가리키는 값: %d\n", *ptr);

    /* 포인터로 값 변경 */
    *ptr = 100;
    printf("변경 후 x: %d\n", x);

    /* 포인터의 포인터 */
    int** pptr = &ptr;
    printf("**pptr = %d\n", **pptr);

    /* swap */
    int a = 1, b = 2;
    swap(&a, &b);
    printf("swap 후: a=%d, b=%d\n", a, b);

    /* 포인터 수정 */
    int* arr = NULL;
    make_array(&arr, 5);
    for (int i = 0; i < 5; i++) {
        printf("arr[%d] = %d\n", i, arr[i]);
    }
    free(arr);
    arr = NULL;

    /* void* 제네릭 */
    int val = 999;
    void* generic = &val;
    print_int(generic);
    printf("\n");

    /* NULL 포인터 검사 */
    int* np = NULL;
    if (np == NULL) {
        printf("NULL 포인터 — 역참조하지 않습니다\n");
    }

    return 0;
}
