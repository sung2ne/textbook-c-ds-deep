// filename: jemalloc_usage.c
// jemalloc 설치 후 컴파일:
// gcc -std=c17 -O2 -o jemalloc_usage jemalloc_usage.c -ljemalloc
// Ubuntu: sudo apt install libjemalloc-dev

#include <stdio.h>
#include <stdlib.h>

// jemalloc 전용 헤더 (설치 경로에 따라 다름)
#ifdef __has_include
#  if __has_include(<jemalloc/jemalloc.h>)
#    include <jemalloc/jemalloc.h>
#    define HAS_JEMALLOC 1
#  endif
#endif

int main(void) {
    // jemalloc이 없으면 시스템 malloc으로 폴백합니다.
    void *ptrs[100];
    for (int i = 0; i < 100; i++) {
        ptrs[i] = malloc((size_t)(i + 1) * 16);
    }
    for (int i = 0; i < 100; i++) {
        free(ptrs[i]);
    }

#ifdef HAS_JEMALLOC
    // jemalloc 통계 출력 (상세한 메모리 사용 현황)
    malloc_stats_print(NULL, NULL, NULL);

    // 특정 통계 조회: 현재 할당된 바이트 수
    size_t allocated, active, resident;
    size_t sz = sizeof(size_t);
    mallctl("stats.allocated", &allocated, &sz, NULL, 0);
    mallctl("stats.active",    &active,    &sz, NULL, 0);
    mallctl("stats.resident",  &resident,  &sz, NULL, 0);

    printf("\nallocated: %zu bytes\n", allocated);
    printf("active:    %zu bytes\n",   active);
    printf("resident:  %zu bytes\n",   resident);
    printf("fragmentation ratio: %.2f%%\n",
           100.0 * (active - allocated) / (double)active);
#else
    printf("jemalloc not available, using system malloc.\n");
#endif

    return 0;
}
