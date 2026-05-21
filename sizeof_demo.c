// filename: sizeof_demo.c
// gcc -std=c17 -o sizeof_demo sizeof_demo.c

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

/* 패딩이 생기는 구조체 */
typedef struct BadLayout {
    char   a;   /* 1바이트 */
    int    b;   /* 4바이트 — a 뒤에 3바이트 패딩 발생 */
    char   c;   /* 1바이트 */
    double d;   /* 8바이트 — c 뒤에 7바이트 패딩 발생 */
} BadLayout;

/* 패딩을 최소화한 구조체 (크기 큰 멤버 먼저) */
typedef struct GoodLayout {
    double d;   /* 8바이트 */
    int    b;   /* 4바이트 */
    char   a;   /* 1바이트 */
    char   c;   /* 1바이트 */
                /* 끝에 2바이트 패딩 */
} GoodLayout;

int main(void) {
    printf("=== 기본 타입 크기 ===\n");
    printf("char:     %zu 바이트\n", sizeof(char));
    printf("short:    %zu 바이트\n", sizeof(short));
    printf("int:      %zu 바이트\n", sizeof(int));
    printf("long:     %zu 바이트\n", sizeof(long));
    printf("long long:%zu 바이트\n", sizeof(long long));
    printf("float:    %zu 바이트\n", sizeof(float));
    printf("double:   %zu 바이트\n", sizeof(double));
    printf("pointer:  %zu 바이트\n", sizeof(void *));

    printf("\n=== 구조체 크기 ===\n");
    printf("BadLayout:  %zu 바이트 (이론상 최솟값: 14)\n", sizeof(BadLayout));
    printf("GoodLayout: %zu 바이트\n", sizeof(GoodLayout));

    printf("\n=== 멤버 오프셋 (BadLayout) ===\n");
    printf("  a: offset %zu\n", offsetof(BadLayout, a));
    printf("  b: offset %zu\n", offsetof(BadLayout, b));
    printf("  c: offset %zu\n", offsetof(BadLayout, c));
    printf("  d: offset %zu\n", offsetof(BadLayout, d));

    printf("\n=== 멤버 오프셋 (GoodLayout) ===\n");
    printf("  d: offset %zu\n", offsetof(GoodLayout, d));
    printf("  b: offset %zu\n", offsetof(GoodLayout, b));
    printf("  a: offset %zu\n", offsetof(GoodLayout, a));
    printf("  c: offset %zu\n", offsetof(GoodLayout, c));

    return 0;
}
