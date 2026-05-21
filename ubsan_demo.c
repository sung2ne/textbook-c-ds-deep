// filename: ubsan_demo.c
// gcc -fsanitize=undefined -g -O1 -o ubsan_demo ubsan_demo.c

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

void signed_overflow(void) {
    int x = INT_MAX;
    int y = x + 1;  /* signed overflow → UB → UBSan이 잡음 */
    printf("y = %d\n", y);
}

void null_dereference(void) {
    int *p = NULL;
    *p = 42;  /* null dereference → UB */
}

void divide_by_zero(void) {
    int x = 10;
    int y = 0;
    printf("%d\n", x / y);  /* divide by zero → UB */
}

void shift_overflow(void) {
    unsigned int x = 1;
    unsigned int y = x << 32;  /* shift by 32 on 32-bit int → UB */
    printf("%u\n", y);
}

void array_oob_local(void) {
    int arr[5];
    arr[10] = 1;  /* local array OOB → UBSan (-fsanitize=bounds 필요) */
}

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    switch (argv[1][0]) {
        case '1': signed_overflow();   break;
        case '2': null_dereference();  break;
        case '3': divide_by_zero();    break;
        case '4': shift_overflow();    break;
    }
    return 0;
}

/*
=== 예상 UBSan 출력 (signed overflow) ===

ubsan_demo.c:9:18: runtime error: signed integer overflow:
2147483647 + 1 cannot be represented in type 'int'
*/
