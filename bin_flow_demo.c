// filename: bin_flow_demo.c
// gcc -std=c17 -O2 -o bin_flow_demo bin_flow_demo.c

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

int main(void) {
    // tcache 실험: 같은 크기 반복 할당/해제
    printf("=== tcache 실험 ===\n");
    void *ptrs[8];
    for (int i = 0; i < 7; i++) {
        ptrs[i] = malloc(32);
    }
    for (int i = 0; i < 7; i++) {
        free(ptrs[i]);   // 7개가 tcache[size_class]에 쌓입니다.
    }
    for (int i = 0; i < 7; i++) {
        ptrs[i] = malloc(32);   // tcache에서 바로 꺼냄
        printf("tcache hit: %p\n", ptrs[i]);
    }
    for (int i = 0; i < 7; i++) free(ptrs[i]);

    // unsorted bin 실험: 큰 청크를 free하면 unsorted bin으로 감
    printf("\n=== unsorted bin 실험 ===\n");
    // 먼저 barrier를 할당해서 top chunk 병합 방지
    void *barrier = malloc(512);
    void *big = malloc(300);
    free(big);   // tcache 범위 초과 → unsorted bin

    // 같은 크기 재요청 → unsorted bin에서 꺼냄
    void *same = malloc(300);
    printf("unsorted bin reuse: %p (same as %p? %s)\n",
           same, big, same == big ? "yes" : "no");

    free(same);
    free(barrier);

    malloc_stats();
    return 0;
}
