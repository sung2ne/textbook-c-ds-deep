// filename: valgrind_demo.c
// gcc -g -O0 -o valgrind_demo valgrind_demo.c
// valgrind --leak-check=full ./valgrind_demo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demo_invalid_read(void) {
    int *arr = malloc(5 * sizeof(int));
    for (int i = 0; i < 5; i++) arr[i] = i;
    /* 범위 밖 읽기 */
    printf("arr[5] = %d\n", arr[5]);  /* Invalid read! */
    free(arr);
}

void demo_uninitialized(void) {
    int *p = malloc(sizeof(int));
    /* 초기화 없이 읽기 */
    if (*p > 0) {  /* Conditional jump on uninitialized value! */
        printf("양수\n");
    }
    free(p);
}

void demo_use_after_free(void) {
    int *p = malloc(sizeof(int));
    *p = 42;
    free(p);
    printf("UAF: %d\n", *p);  /* Invalid read of freed memory! */
}

void demo_memory_leak(void) {
    int *p = malloc(100 * sizeof(int));
    memset(p, 0, 100 * sizeof(int));
    /* free(p) 없음 → 메모리 누수 */
}

int main(void) {
    demo_invalid_read();
    demo_uninitialized();
    demo_use_after_free();
    demo_memory_leak();
    return 0;
}

/*
=== 예상 Valgrind 출력 ===

==12345== Invalid read of size 4
==12345==    at 0x401234: demo_invalid_read (valgrind_demo.c:11)
==12345==    at 0x401456: main (valgrind_demo.c:36)
==12345==  Address 0x5204054 is 0 bytes after a block of size 20 alloc'd
==12345==    at 0x4848899: malloc (vg_replace_malloc.c:381)

==12345== Conditional jump or move depends on uninitialised value(s)
==12345==    at 0x401267: demo_uninitialized (valgrind_demo.c:19)

==12345== Invalid read of size 4
==12345==    at 0x401312: demo_use_after_free (valgrind_demo.c:28)
==12345==  Address 0x5204054 is 0 bytes inside a block of size 4 free'd
==12345==    at 0x484B27F: free (vg_replace_malloc.c:872)

==12345== LEAK SUMMARY:
==12345==    definitely lost: 400 bytes in 1 blocks
==12345==    indirectly lost: 0 bytes in 0 blocks
*/
