// filename: dynamic_array.c
// gcc -std=c17 -O2 -o dynamic_array dynamic_array.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int  *data;
    int   size;
    int   capacity;
    int   total_copies;   /* 총 복사 횟수 추적 */
} DynArray;

static void da_init(DynArray *a) {
    a->capacity     = 1;
    a->size         = 0;
    a->total_copies = 0;
    a->data         = malloc(sizeof(int) * a->capacity);
}

static void da_push(DynArray *a, int val) {
    if (a->size == a->capacity) {
        int new_cap = a->capacity * 2;
        int *new_data = malloc(sizeof(int) * new_cap);
        /* 기존 원소 복사 */
        memcpy(new_data, a->data, sizeof(int) * a->size);
        a->total_copies += a->size;   /* 복사 횟수 누적 */
        printf("  [확장] %d -> %d, 이번 복사 %d개, 누적 %d개\n",
               a->capacity, new_cap, a->size, a->total_copies);
        free(a->data);
        a->data     = new_data;
        a->capacity = new_cap;
    }
    a->data[a->size++] = val;
}

static void da_free(DynArray *a) {
    free(a->data);
}

int main(void) {
    DynArray a;
    da_init(&a);

    int N = 32;
    printf("push_back %d회 수행:\n", N);
    for (int i = 0; i < N; i++) da_push(&a, i);

    printf("\n총 push_back: %d회\n", N);
    printf("총 원소 복사: %d회\n", a.total_copies);
    printf("회당 평균 복사: %.2f회\n", (double)a.total_copies / N);

    da_free(&a);
    return 0;
}
