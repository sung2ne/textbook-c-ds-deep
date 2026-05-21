// filename: slab_allocator.c
// gcc -std=c17 -O2 -o slab_allocator slab_allocator.c

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <assert.h>

// ──────────────────────────────────────────────
// 단순화된 사용자 공간 Slab Cache 구현
// ──────────────────────────────────────────────
// 구조: slab(page 크기 블록)을 mmap으로 가져오고
//        그 안에서 고정 크기 객체를 bitmap으로 관리합니다.
//        (단순화를 위해 malloc으로 구현)

#define SLAB_PAGE_SIZE (4096)

typedef struct SlabCache SlabCache;

typedef struct Slab {
    struct Slab *next;      // 다음 slab (partial list)
    uint8_t     *memory;    // 객체 저장 공간
    uint64_t     bitmap;    // 사용 중인 슬롯 비트맵 (최대 64 슬롯)
    int          free_count;// 사용 가능한 슬롯 수
    SlabCache   *cache;     // 소속 캐시 (역참조)
} Slab;

struct SlabCache {
    const char  *name;       // 캐시 이름
    size_t       obj_size;   // 객체 크기
    size_t       alignment;  // 객체 정렬
    int          objs_per_slab; // slab당 객체 수
    Slab        *partial;    // 일부 사용 중인 slab 목록
    Slab        *full;       // 꽉 찬 slab 목록
    size_t       total_allocated;  // 통계
    size_t       total_freed;      // 통계
};

// slab 하나를 생성합니다.
static Slab *slab_create(SlabCache *cache) {
    Slab *slab = (Slab*)malloc(sizeof(Slab));
    if (!slab) return NULL;

    size_t mem_size = (size_t)cache->objs_per_slab * cache->obj_size;
    slab->memory = (uint8_t*)malloc(mem_size);
    if (!slab->memory) { free(slab); return NULL; }

    slab->bitmap     = 0;           // 모든 슬롯 사용 가능 (0=free, 1=used)
    slab->free_count = cache->objs_per_slab;
    slab->next       = NULL;
    slab->cache      = cache;
    return slab;
}

// SlabCache 초기화
int slab_cache_init(SlabCache *cache, const char *name,
                    size_t obj_size, size_t alignment) {
    if (alignment == 0) alignment = sizeof(void*);
    // obj_size를 alignment 배수로 올림
    obj_size = (obj_size + alignment - 1) & ~(alignment - 1);
    if (obj_size < sizeof(void*)) obj_size = sizeof(void*);

    cache->name      = name;
    cache->obj_size  = obj_size;
    cache->alignment = alignment;
    cache->partial   = NULL;
    cache->full      = NULL;
    cache->total_allocated = 0;
    cache->total_freed     = 0;

    // slab당 객체 수: page 크기 기준, 최대 64개 (bitmap 64비트 제한)
    int n = (int)(SLAB_PAGE_SIZE / obj_size);
    if (n > 64) n = 64;
    if (n < 1)  n = 1;
    cache->objs_per_slab = n;

    // 첫 번째 slab 미리 생성
    cache->partial = slab_create(cache);
    return cache->partial ? 0 : -1;
}

// 첫 번째 free 비트를 찾아 인덱스를 반환합니다.
static int find_free_slot(uint64_t bitmap, int count) {
    for (int i = 0; i < count; i++) {
        if (!((bitmap >> i) & 1)) return i;
    }
    return -1;
}

// O(1) 할당: partial slab의 bitmap에서 free 슬롯 찾기
void *slab_cache_alloc(SlabCache *cache) {
    // partial slab이 없으면 새로 만듭니다.
    if (!cache->partial) {
        cache->partial = slab_create(cache);
        if (!cache->partial) return NULL;
    }

    Slab *slab = cache->partial;
    int slot = find_free_slot(slab->bitmap, cache->objs_per_slab);
    assert(slot >= 0);

    slab->bitmap |= (1ULL << slot);
    slab->free_count--;
    cache->total_allocated++;

    // slab이 꽉 찼으면 full 리스트로 이동
    if (slab->free_count == 0) {
        cache->partial = slab->next;
        slab->next     = cache->full;
        cache->full    = slab;
    }

    return slab->memory + (size_t)slot * cache->obj_size;
}

// O(1) 해제: 속한 slab의 bitmap 비트를 지웁니다.
void slab_cache_free(SlabCache *cache, void *obj) {
    if (!obj) return;
    cache->total_freed++;

    // 어느 slab에 속하는지 full 리스트에서 탐색합니다.
    // (단순화 구현 — 실제 SLUB은 page 메타데이터로 O(1) 역추적)
    Slab **prev = &cache->full;
    Slab *slab = cache->full;
    while (slab) {
        uint8_t *start = slab->memory;
        uint8_t *end   = start + (size_t)cache->objs_per_slab * cache->obj_size;
        if ((uint8_t*)obj >= start && (uint8_t*)obj < end) {
            int slot = (int)(((uint8_t*)obj - start) / cache->obj_size);
            slab->bitmap     &= ~(1ULL << slot);
            slab->free_count++;
            // full → partial 이동
            *prev          = slab->next;
            slab->next     = cache->partial;
            cache->partial = slab;
            return;
        }
        prev = &slab->next;
        slab = slab->next;
    }

    // partial 리스트에서 탐색
    prev = &cache->partial;
    slab = cache->partial;
    while (slab) {
        uint8_t *start = slab->memory;
        uint8_t *end   = start + (size_t)cache->objs_per_slab * cache->obj_size;
        if ((uint8_t*)obj >= start && (uint8_t*)obj < end) {
            int slot = (int)(((uint8_t*)obj - start) / cache->obj_size);
            slab->bitmap     &= ~(1ULL << slot);
            slab->free_count++;
            return;
        }
        prev = &slab->next;
        slab = slab->next;
    }

    fprintf(stderr, "slab_cache_free: unknown pointer %p\n", obj);
}

// 통계 출력
void slab_cache_stats(const SlabCache *cache) {
    printf("SlabCache '%s': obj_size=%zu, objs_per_slab=%d, "
           "allocated=%zu, freed=%zu, in_use=%zu\n",
           cache->name, cache->obj_size, cache->objs_per_slab,
           cache->total_allocated, cache->total_freed,
           cache->total_allocated - cache->total_freed);
}

// ──────────────────────────────────────────────
// 예제: 프로세스 디스크립터 시뮬레이션
// ──────────────────────────────────────────────

typedef struct ProcessDescriptor {
    int     pid;
    int     ppid;
    char    name[64];
    uint64_t start_time;
    int     state;          // 0=running, 1=sleeping, 2=zombie
    int     exit_code;
} ProcessDescriptor;

static double elapsed_ns(struct timespec s, struct timespec e) {
    return (e.tv_sec - s.tv_sec) * 1e9 + (e.tv_nsec - s.tv_nsec);
}

int main(void) {
    SlabCache proc_cache;
    if (slab_cache_init(&proc_cache, "process_descriptor",
                        sizeof(ProcessDescriptor),
                        _Alignof(ProcessDescriptor)) != 0) {
        fprintf(stderr, "slab_cache_init failed\n");
        return 1;
    }

    printf("=== Slab Allocator 기능 테스트 ===\n");

    // 프로세스 생성 시뮬레이션
    ProcessDescriptor *procs[10];
    for (int i = 0; i < 10; i++) {
        procs[i] = slab_cache_alloc(&proc_cache);
        procs[i]->pid   = 1000 + i;
        procs[i]->ppid  = 1;
        procs[i]->state = 0;
        snprintf(procs[i]->name, 64, "process_%d", i);
    }

    printf("10개 프로세스 생성 완료\n");
    slab_cache_stats(&proc_cache);

    // 프로세스 일부 종료 (zombie 상태 후 해제)
    for (int i = 0; i < 5; i++) {
        procs[i]->state     = 2;  // zombie
        procs[i]->exit_code = 0;
        slab_cache_free(&proc_cache, procs[i]);
    }

    printf("\n5개 프로세스 종료 후:\n");
    slab_cache_stats(&proc_cache);

    // 재할당 — slab 재사용
    ProcessDescriptor *new_proc = slab_cache_alloc(&proc_cache);
    new_proc->pid   = 2000;
    new_proc->state = 0;
    snprintf(new_proc->name, 64, "new_process");
    printf("\n새 프로세스 할당: pid=%d, addr=%p\n", new_proc->pid, (void*)new_proc);
    slab_cache_free(&proc_cache, new_proc);

    // ──────────────────────────────────────────
    // 성능 벤치마크
    // ──────────────────────────────────────────
    printf("\n=== 성능 벤치마크 ===\n");
    const int ITER = 10000;
    struct timespec t1, t2;

    // Slab 방식
    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (int i = 0; i < ITER; i++) {
        ProcessDescriptor *p = slab_cache_alloc(&proc_cache);
        p->pid = i;
        slab_cache_free(&proc_cache, p);
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);
    printf("Slab alloc+free x%d: %.1f ns/op\n",
           ITER, elapsed_ns(t1, t2) / ITER);

    // 시스템 malloc 방식
    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (int i = 0; i < ITER; i++) {
        ProcessDescriptor *p = malloc(sizeof(ProcessDescriptor));
        p->pid = i;
        free(p);
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);
    printf("malloc+free x%d: %.1f ns/op\n",
           ITER, elapsed_ns(t1, t2) / ITER);

    // 나머지 5개 해제
    for (int i = 5; i < 10; i++) {
        slab_cache_free(&proc_cache, procs[i]);
    }

    slab_cache_stats(&proc_cache);
    return 0;
}
