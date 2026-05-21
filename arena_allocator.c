// filename: arena_allocator.c
// gcc -std=c17 -O2 -o arena_allocator arena_allocator.c

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>

// ──────────────────────────────────────────────
// Arena Allocator 구현
// ──────────────────────────────────────────────

typedef struct Arena {
    uint8_t *memory;    // 관리할 메모리 블록
    size_t   capacity;  // 총 크기 (바이트)
    size_t   used;      // 현재 사용 위치 (bump pointer)
} Arena;

// 정렬 헬퍼: size를 align의 배수로 올림
static size_t align_up(size_t size, size_t align) {
    return (size + align - 1) & ~(align - 1);
}

// Arena 초기화
int arena_init(Arena *arena, size_t capacity) {
    arena->memory   = (uint8_t*)malloc(capacity);
    if (!arena->memory) return -1;
    arena->capacity = capacity;
    arena->used     = 0;
    return 0;
}

// O(1) 할당: bump pointer를 앞으로 밀기만 합니다.
// align: 정렬 단위 (일반적으로 8 또는 16)
void *arena_alloc(Arena *arena, size_t size, size_t align) {
    size_t aligned_used = align_up(arena->used, align);
    if (aligned_used + size > arena->capacity) {
        return NULL;  // 공간 부족
    }
    void *ptr = arena->memory + aligned_used;
    arena->used = aligned_used + size;
    return ptr;
}

// 전체 해제: used를 0으로 되돌립니다.
void arena_reset(Arena *arena) {
    arena->used = 0;
}

// Arena 상태 출력
void arena_stats(const Arena *arena) {
    printf("Arena: capacity=%zu, used=%zu (%.1f%%), free=%zu\n",
           arena->capacity, arena->used,
           100.0 * arena->used / arena->capacity,
           arena->capacity - arena->used);
}

// Arena 소멸
void arena_destroy(Arena *arena) {
    free(arena->memory);
    arena->memory   = NULL;
    arena->capacity = 0;
    arena->used     = 0;
}

// ──────────────────────────────────────────────
// 예제: HTTP 요청 처리 시뮬레이션
// ──────────────────────────────────────────────
// Nginx 스타일: 요청별 arena → 요청 완료 시 reset()

typedef struct HttpRequest {
    char   method[8];
    char   path[256];
    char   body[1024];
    int    status_code;
} HttpRequest;

typedef struct HttpHeader {
    char key[64];
    char value[256];
} HttpHeader;

static double elapsed_us(struct timespec s, struct timespec e) {
    return (e.tv_sec - s.tv_sec) * 1e6
         + (e.tv_nsec - s.tv_nsec) / 1e3;
}

// 요청 하나를 처리하는 함수 (arena에서만 할당)
static void handle_request(Arena *arena, int req_id) {
    // 요청 구조체 할당
    HttpRequest *req = arena_alloc(arena, sizeof(HttpRequest), _Alignof(HttpRequest));
    if (!req) return;

    snprintf(req->method, sizeof(req->method), "GET");
    snprintf(req->path, sizeof(req->path), "/api/items/%d", req_id);
    req->status_code = 200;

    // 헤더 배열 할당
    const int NUM_HEADERS = 4;
    HttpHeader *headers = arena_alloc(arena,
                                       sizeof(HttpHeader) * NUM_HEADERS,
                                       _Alignof(HttpHeader));
    if (!headers) return;

    snprintf(headers[0].key, 64, "Content-Type");
    snprintf(headers[0].value, 256, "application/json");
    snprintf(headers[1].key, 64, "X-Request-Id");
    snprintf(headers[1].value, 256, "%d", req_id);

    // 응답 버퍼 할당
    char *response = arena_alloc(arena, 512, 1);
    if (!response) return;

    snprintf(response, 512,
             "{\"id\":%d,\"method\":\"%s\",\"path\":\"%s\",\"status\":%d}",
             req_id, req->method, req->path, req->status_code);

    // 처리 완료 — 개별 free() 없음!
    // (실제 Nginx는 요청 완료 이벤트에서 pool을 해제합니다)
}

int main(void) {
    Arena arena;
    if (arena_init(&arena, 64 * 1024) != 0) {  // 64KB arena
        fprintf(stderr, "arena_init failed\n");
        return 1;
    }

    printf("=== HTTP 요청 시뮬레이션 ===\n");

    const int NUM_REQUESTS = 100;
    struct timespec t1, t2;

    // Arena 방식: 각 요청마다 arena 사용 후 reset
    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (int i = 0; i < NUM_REQUESTS; i++) {
        handle_request(&arena, i);
        arena_reset(&arena);   // 요청 끝 → 한 번에 초기화
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);

    printf("Arena 방식: %d 요청 처리 시간 = %.1f us\n",
           NUM_REQUESTS, elapsed_us(t1, t2));

    arena_stats(&arena);

    // ──────────────────────────────────────────
    // 비교: 시스템 malloc/free 방식
    // ──────────────────────────────────────────
    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (int i = 0; i < NUM_REQUESTS; i++) {
        HttpRequest *req = malloc(sizeof(HttpRequest));
        HttpHeader  *hdr = malloc(sizeof(HttpHeader) * 4);
        char        *buf = malloc(512);

        snprintf(req->method, 8, "GET");
        req->status_code = 200;

        free(req);
        free(hdr);
        free(buf);
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);

    printf("malloc/free 방식: %d 요청 처리 시간 = %.1f us\n",
           NUM_REQUESTS, elapsed_us(t1, t2));

    arena_destroy(&arena);

    // ──────────────────────────────────────────
    // 정렬 동작 확인
    // ──────────────────────────────────────────
    printf("\n=== 정렬 동작 확인 ===\n");
    Arena align_test;
    arena_init(&align_test, 1024);

    char     *c1 = arena_alloc(&align_test, 1, 1);   // 1바이트 정렬
    double   *d1 = arena_alloc(&align_test, sizeof(double), _Alignof(double));
    int      *i1 = arena_alloc(&align_test, sizeof(int),    _Alignof(int));

    printf("char   at %p (offset %zu)\n",
           (void*)c1, (size_t)(c1 - (char*)align_test.memory));
    printf("double at %p (offset %zu, aligned to 8: %s)\n",
           (void*)d1, (size_t)((char*)d1 - (char*)align_test.memory),
           ((uintptr_t)d1 % 8 == 0) ? "yes" : "no");
    printf("int    at %p (offset %zu, aligned to 4: %s)\n",
           (void*)i1, (size_t)((char*)i1 - (char*)align_test.memory),
           ((uintptr_t)i1 % 4 == 0) ? "yes" : "no");

    arena_destroy(&align_test);
    return 0;
}
