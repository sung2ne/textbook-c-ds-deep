// filename: chunk_inspect.c
// gcc -std=c17 -O2 -o chunk_inspect chunk_inspect.c

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// glibc 내부 구조를 직접 보는 방법 (비공개 API, 학습 목적)
// malloc_chunk 구조체를 직접 정의합니다.
struct malloc_chunk {
    size_t prev_size;
    size_t size;
    struct malloc_chunk *fd;
    struct malloc_chunk *bk;
};

int main(void) {
    void *p = malloc(100);

    // malloc이 반환한 포인터에서 16바이트 앞이 chunk 헤더입니다.
    struct malloc_chunk *chunk = (struct malloc_chunk *)((char *)p - 16);

    printf("User pointer: %p\n", p);
    printf("Chunk pointer: %p\n", (void*)chunk);
    printf("Chunk size field: 0x%zx (%zu bytes)\n",
           chunk->size, chunk->size & ~0x7UL);
    printf("PREV_INUSE flag: %s\n", (chunk->size & 1) ? "set" : "clear");
    printf("IS_MMAPPED flag: %s\n", (chunk->size & 2) ? "set" : "clear");
    printf("NON_MAIN_ARENA flag: %s\n", (chunk->size & 4) ? "set" : "clear");

    free(p);
    return 0;
}
