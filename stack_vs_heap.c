// filename: stack_vs_heap.c
// gcc -std=c17 -O2 -o stack_vs_heap stack_vs_heap.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 스택에 할당되는 예제 */
int add(int x, int y) {
    int result = x + y;   /* result는 스택에 있습니다 */
    return result;
    /* 여기서 함수가 끝나면 result는 사라집니다 */
}

/* 스택 변수의 주소를 반환하면 — 위험! */
int* dangerous_ptr(void) {
    int local = 42;
    return &local;   /* 경고: 스택 메모리 주소 반환 */
    /* 함수 종료 후 local은 이미 사라졌습니다 */
}

/* 힙에 할당되는 예제 */
int* heap_alloc(int value) {
    int* ptr = malloc(sizeof(int));   /* 힙에 4바이트 요청 */
    if (ptr == NULL) {
        fprintf(stderr, "malloc 실패\n");
        exit(EXIT_FAILURE);
    }
    *ptr = value;
    return ptr;   /* 힙 주소를 반환 — 안전합니다 */
    /* free()를 호출하기 전까지 이 메모리는 살아있습니다 */
}

/* 스택 오버플로우 발생 */
void infinite_recursion(int depth) {
    char buffer[1024];   /* 매 호출마다 1KB 스택 사용 */
    buffer[0] = depth;   /* 컴파일러 최적화 방지 */
    printf("depth: %d\n", depth);
    infinite_recursion(depth + 1);   /* 끝없이 재귀 */
}

int main(void) {
    /* 스택 변수 */
    int stack_var = 100;
    printf("[스택] stack_var 주소: %p, 값: %d\n",
           (void*)&stack_var, stack_var);

    /* 함수 호출 스택 */
    int sum = add(10, 20);
    printf("[스택] add(10, 20) = %d\n", sum);

    /* 힙 변수 */
    int* heap_var = heap_alloc(200);
    printf("[힙]   heap_var 주소: %p, 값: %d\n",
           (void*)heap_var, *heap_var);

    /* 반드시 free! */
    free(heap_var);
    heap_var = NULL;   /* 댕글링 포인터 방지 */

    /* 위험한 스택 포인터 반환 */
    int* danger = dangerous_ptr();
    /* danger가 가리키는 메모리는 이미 재사용될 수 있습니다 */
    /* 아래 printf는 아무 값이나 출력될 수 있습니다 (UB) */
    printf("[위험] 댕글링 포인터 값: %d (undefined behavior)\n", *danger);

    /* 스택 오버플로우 테스트 — 실행하면 segfault 발생 */
    /* infinite_recursion(0); */

    return 0;
}
