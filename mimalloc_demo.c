// filename: mimalloc_demo.c
// mimalloc 설치: https://github.com/microsoft/mimalloc
// Ubuntu: sudo apt install libmimalloc-dev
// gcc -std=c17 -O2 -o mimalloc_demo mimalloc_demo.c -lmimalloc

#include <stdio.h>
#include <stdlib.h>

#ifdef __has_include
#  if __has_include(<mimalloc.h>)
#    include <mimalloc.h>
#    define USE_MIMALLOC 1
#  endif
#endif

int main(void) {
#ifdef USE_MIMALLOC
    // mimalloc 전용 API: mi_malloc은 malloc과 동일하게 사용 가능
    void *p = mi_malloc(256);
    printf("mimalloc allocated: %p\n", p);

    // 통계 출력
    mi_stats_print(NULL);

    mi_free(p);

    // 옵션 설정: 할당 시 초기화 패턴 활성화 (디버그용)
    // mi_option_set(mi_option_initialize_zero, 1);
#else
    // mimalloc 없으면 일반 malloc 사용
    void *p = malloc(256);
    printf("system malloc allocated: %p\n", p);
    free(p);
    printf("mimalloc not available.\n");
#endif
    return 0;
}
