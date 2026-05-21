// filename: asan_demo.c
// gcc -fsanitize=address -g -O1 -o asan_demo asan_demo.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void heap_buffer_overflow(void) {
    int *arr = malloc(5 * sizeof(int));
    arr[5] = 99;  /* heap buffer overflow → ASan이 즉시 잡음 */
    free(arr);
}

void stack_buffer_overflow(void) {
    int arr[5];
    arr[6] = 99;  /* stack buffer overflow → ASan이 잡음 */
}

void use_after_free(void) {
    int *p = malloc(sizeof(int));
    *p = 42;
    free(p);
    *p = 100;  /* UAF → ASan이 잡음 */
}

void double_free(void) {
    int *p = malloc(sizeof(int));
    free(p);
    free(p);  /* double free → ASan이 잡음 */
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("사용법: %s [1|2|3|4]\n", argv[0]);
        printf("  1: heap buffer overflow\n");
        printf("  2: stack buffer overflow\n");
        printf("  3: use-after-free\n");
        printf("  4: double free\n");
        return 1;
    }
    switch (argv[1][0]) {
        case '1': heap_buffer_overflow();  break;
        case '2': stack_buffer_overflow(); break;
        case '3': use_after_free();        break;
        case '4': double_free();           break;
    }
    return 0;
}

/*
=== 예상 ASan 출력 (heap buffer overflow) ===

=================================================================
==12345==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x602000000034
READ of size 4 at 0x602000000034 thread T0
    #0 0x401234 in heap_buffer_overflow asan_demo.c:9
    #1 0x401456 in main asan_demo.c:28

0x602000000034 is located 0 bytes to the right of 20-byte region [0x602000000020,0x602000000034)
allocated by thread T0 here:
    #0 0x7f8b malloc
    #1 0x401223 in heap_buffer_overflow asan_demo.c:8

Shadow bytes around the buggy address:
  0x0c047fff8000: fa fa 00 00 00 fa fa fa ...
  =>0x0c047fff8006:[fa]...       ← fa = heap right redzone
*/
