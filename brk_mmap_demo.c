// filename: brk_mmap_demo.c
// gcc -std=c17 -O2 -o brk_mmap_demo brk_mmap_demo.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    // 소형 할당: brk로 힙 확장 (MMAP_THRESHOLD=128KB 이하)
    void *small = malloc(1024);
    printf("small alloc: %p\n", small);

    // 대형 할당: mmap 사용 (128KB 초과)
    void *large = malloc(200 * 1024);
    printf("large alloc: %p\n", large);

    // 주소 차이를 보면 small은 힙 영역(낮은 주소),
    // large는 mmap 영역(높은 주소 또는 다른 영역)에 있습니다.
    printf("address diff: %td bytes\n", (char*)large - (char*)small);

    free(small);
    free(large);
    return 0;
}
