// filename: embedded_pool.c
// ARM Cortex-M에서 사용 가능한 순수 정적 Pool Allocator
// gcc -std=c17 -O2 -o embedded_pool embedded_pool.c

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define POOL_CAPACITY 64
#define OBJECT_SIZE   32

// 전역 배열로 backing store를 만듭니다.
// 임베디드에서는 BSS 세그먼트에 배치됩니다.
static uint8_t g_pool_memory[POOL_CAPACITY][OBJECT_SIZE];
static void   *g_free_list = NULL;
static int     g_initialized = 0;

static void pool_static_init(void) {
    for (int i = 0; i < POOL_CAPACITY - 1; i++) {
        *(void**)g_pool_memory[i] = g_pool_memory[i + 1];
    }
    *(void**)g_pool_memory[POOL_CAPACITY - 1] = NULL;
    g_free_list = g_pool_memory[0];
    g_initialized = 1;
}

void *pool_static_alloc(void) {
    if (!g_initialized) pool_static_init();
    if (!g_free_list) return NULL;
    void *obj = g_free_list;
    g_free_list = *(void**)obj;
    return obj;
}

void pool_static_free(void *obj) {
    *(void**)obj = g_free_list;
    g_free_list = obj;
}

// FreeRTOS의 pvPortMalloc도 이와 유사한 구조입니다.
// heap_4.c: first-fit 방식, heap_2.c: fixed-size block pool 방식

int main(void) {
    void *p1 = pool_static_alloc();
    void *p2 = pool_static_alloc();
    printf("p1=%p, p2=%p\n", p1, p2);

    pool_static_free(p1);
    void *p3 = pool_static_alloc();
    printf("p3=%p (p1 재사용: %s)\n", p3, p3 == p1 ? "yes" : "no");

    pool_static_free(p2);
    pool_static_free(p3);
    return 0;
}
