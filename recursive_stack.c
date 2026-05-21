// filename: recursive_stack.c
// gcc -std=c17 -g -o recursive_stack recursive_stack.c

#include <stdio.h>

/* 재귀 깊이를 출력하는 함수 */
static int factorial(int n, int depth) {
    printf("depth %d: n=%d (스택 사용)\n", depth, n);
    if (n <= 1) return 1;
    return n * factorial(n - 1, depth + 1);
}

int main(void) {
    printf("factorial(5) = %d\n\n", factorial(5, 0));
    return 0;
}
