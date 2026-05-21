// filename: memory_mistakes.c
// gcc -std=c17 -g -fsanitize=address -o memory_mistakes memory_mistakes.c
// (ASan으로 실행하면 각 버그를 잡을 수 있습니다)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* double free 시연 (실행 시 crash) */
void demo_double_free(void) {
    int* p = malloc(sizeof(int));
    if (!p) return;
    *p = 42;
    free(p);
    p = NULL;   /* 이렇게 하면 두 번째 free(NULL)은 안전 */
    free(p);    /* free(NULL)은 noop — OK */
    printf("double free 안전하게 처리됨\n");
}

/* UAF 수정 버전 */
void demo_uaf_fixed(void) {
    int* p = malloc(sizeof(int));
    if (!p) return;
    *p = 42;
    printf("free 전: %d\n", *p);
    free(p);
    p = NULL;
    /* p = NULL이므로 역참조하면 segfault — 실수를 즉시 알 수 있음 */
    printf("UAF 수정됨 (p = NULL)\n");
}

/* OOB 안전 버전 */
void demo_oob_safe(void) {
    int arr[5] = {10, 20, 30, 40, 50};
    int n = 5;
    for (int i = 0; i < n; i++) {   /* i < n 조건으로 경계 보장 */
        printf("arr[%d] = %d\n", i, arr[i]);
    }
}

/* 누수 없는 버전 */
void demo_no_leak(int n) {
    int* arr = malloc(sizeof(int) * (size_t)n);
    if (arr == NULL) return;

    for (int i = 0; i < n; i++) arr[i] = i;

    if (n > 100) {
        free(arr);   /* 모든 경로에서 free */
        return;
    }

    /* 처리 */
    printf("합: %d\n", arr[0] + arr[n-1]);

    free(arr);   /* 정상 경로에서도 free */
}

int main(void) {
    demo_double_free();
    demo_uaf_fixed();
    demo_oob_safe();
    demo_no_leak(5);
    demo_no_leak(200);
    return 0;
}
