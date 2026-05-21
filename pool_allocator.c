// filename: pool_allocator.c
// gcc -std=c17 -O2 -o pool_allocator pool_allocator.c

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>

// ──────────────────────────────────────────────
// Pool Allocator 구현
// ──────────────────────────────────────────────

typedef struct PoolAllocator {
    uint8_t  *memory;      // 관리할 메모리 블록
    void     *free_list;   // 사용 가능한 슬롯 연결 리스트 head
    size_t    obj_size;    // 각 슬롯의 크기 (바이트)
    size_t    capacity;    // 총 슬롯 수
    size_t    allocated;   // 현재 할당된 슬롯 수 (통계)
} PoolAllocator;

// Pool 초기화: capacity개의 obj_size 크기 슬롯을 준비합니다.
int pool_init(PoolAllocator *pool, size_t obj_size, size_t capacity) {
    // 각 슬롯은 최소 포인터 크기 이상이어야 합니다.
    if (obj_size < sizeof(void*)) {
        obj_size = sizeof(void*);
    }

    pool->obj_size  = obj_size;
    pool->capacity  = capacity;
    pool->allocated = 0;
    pool->memory    = (uint8_t*)malloc(obj_size * capacity);
    if (!pool->memory) return -1;

    // free list 초기화: 각 슬롯의 첫 8바이트가 다음 슬롯을 가리킵니다.
    pool->free_list = pool->memory;
    for (size_t i = 0; i < capacity - 1; i++) {
        void **slot = (void**)(pool->memory + i * obj_size);
        *slot = pool->memory + (i + 1) * obj_size;
    }
    // 마지막 슬롯은 NULL로 끝납니다.
    void **last = (void**)(pool->memory + (capacity - 1) * obj_size);
    *last = NULL;

    return 0;
}

// O(1) 할당: free_list의 head를 꺼냅니다.
void *pool_alloc(PoolAllocator *pool) {
    if (!pool->free_list) return NULL;   // 풀이 가득 참

    void *obj = pool->free_list;
    pool->free_list = *(void**)obj;      // head를 다음 슬롯으로 이동
    pool->allocated++;

    // 디버그 모드: 초기화 패턴으로 채워 use-after-free 탐지 도움
    memset(obj, 0xAB, pool->obj_size);
    return obj;
}

// O(1) 해제: 반환된 슬롯을 free_list의 head로 추가합니다.
void pool_free(PoolAllocator *pool, void *obj) {
    if (!obj) return;

    // 디버그 모드: 해제 패턴으로 채워 dangling pointer 탐지 도움
    memset(obj, 0xCD, pool->obj_size);

    *(void**)obj = pool->free_list;
    pool->free_list = obj;
    pool->allocated--;
}

// Pool 통계 출력
void pool_stats(const PoolAllocator *pool) {
    printf("Pool: obj_size=%zu, capacity=%zu, allocated=%zu, free=%zu\n",
           pool->obj_size, pool->capacity,
           pool->allocated, pool->capacity - pool->allocated);
}

// Pool 소멸
void pool_destroy(PoolAllocator *pool) {
    free(pool->memory);
    pool->memory    = NULL;
    pool->free_list = NULL;
}

// ──────────────────────────────────────────────
// 예제: 게임 파티클 시스템
// ──────────────────────────────────────────────

typedef struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float lifetime;
    uint32_t color;
} Particle;

static double elapsed_ns(struct timespec s, struct timespec e) {
    return (e.tv_sec - s.tv_sec) * 1e9 + (e.tv_nsec - s.tv_nsec);
}

int main(void) {
    PoolAllocator pool;
    if (pool_init(&pool, sizeof(Particle), 10000) != 0) {
        fprintf(stderr, "pool_init failed\n");
        return 1;
    }

    printf("=== Pool Allocator 기능 테스트 ===\n");

    // 파티클 할당
    Particle *p1 = pool_alloc(&pool);
    Particle *p2 = pool_alloc(&pool);
    p1->x = 1.0f; p1->y = 2.0f; p1->lifetime = 3.0f;
    p2->x = 4.0f; p2->y = 5.0f; p2->lifetime = 1.5f;

    printf("p1: x=%.1f y=%.1f lifetime=%.1f\n", p1->x, p1->y, p1->lifetime);
    printf("p2: x=%.1f y=%.1f lifetime=%.1f\n", p2->x, p2->y, p2->lifetime);
    pool_stats(&pool);

    // 파티클 해제 후 재사용
    pool_free(&pool, p1);
    Particle *p3 = pool_alloc(&pool);  // p1의 메모리를 재사용
    printf("p3 == p1 주소 재사용: %s\n", p3 == p1 ? "yes" : "no");

    pool_free(&pool, p2);
    pool_free(&pool, p3);

    // ──────────────────────────────────────────
    // 성능 벤치마크: Pool vs malloc
    // ──────────────────────────────────────────
    printf("\n=== 성능 벤치마크 ===\n");
    const int ITER = 100000;
    Particle **ptrs = malloc(ITER * sizeof(Particle*));
    struct timespec t1, t2;

    // Pool Allocator 성능
    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (int i = 0; i < ITER; i++) ptrs[i] = pool_alloc(&pool);
    for (int i = 0; i < ITER; i++) pool_free(&pool, ptrs[i]);
    clock_gettime(CLOCK_MONOTONIC, &t2);
    printf("Pool alloc+free x%d: %.1f ns/op\n",
           ITER, elapsed_ns(t1, t2) / ITER);

    // 시스템 malloc 성능
    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (int i = 0; i < ITER; i++) ptrs[i] = malloc(sizeof(Particle));
    for (int i = 0; i < ITER; i++) free(ptrs[i]);
    clock_gettime(CLOCK_MONOTONIC, &t2);
    printf("System malloc+free x%d: %.1f ns/op\n",
           ITER, elapsed_ns(t1, t2) / ITER);

    free(ptrs);
    pool_destroy(&pool);
    return 0;
}
