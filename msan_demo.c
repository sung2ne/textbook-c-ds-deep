// filename: msan_demo.c
// clang -fsanitize=memory -fno-omit-frame-pointer -g -O1 -o msan_demo msan_demo.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void uninitialized_heap(void) {
    int *arr = malloc(5 * sizeof(int));
    /* memset 또는 초기화 없음 */
    printf("합계: %d\n", arr[0] + arr[1]);  /* MSan이 잡음 */
    free(arr);
}

void uninitialized_stack(void) {
    int x;
    if (x > 0) {  /* x 초기화 안 됨 → MSan이 잡음 */
        printf("양수\n");
    }
}

void propagation_example(void) {
    int uninit;
    int result = uninit * 2;   /* 초기화 안 된 값 전파 */
    if (result > 100) {        /* 이 시점에 MSan 보고 */
        printf("크다\n");
    }
}

int main(void) {
    uninitialized_heap();
    return 0;
}

/*
=== 예상 MSan 출력 ===

Uninitialized bytes in __interceptor_printf at offset 0 inside [0x7f..., 4)
==12345==WARNING: MemorySanitizer: use-of-uninitialized-value
    #0 0x401234 in uninitialized_heap msan_demo.c:11
    #1 0x401456 in main msan_demo.c:22

  Uninitialized value was created by a heap allocation
    #0 0x401223 in malloc
    #1 0x401212 in uninitialized_heap msan_demo.c:8
*/
