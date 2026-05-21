// filename: growth_factor_bench.c
// gcc -std=c17 -O2 -o growth_factor_bench growth_factor_bench.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PUSH_COUNT 10000000UL  /* 1000만 번 push */

typedef struct {
    int*   data;
    size_t size;
    size_t capacity;
} IntVec;

static void vec_init(IntVec* v, size_t init_cap) {
    v->data = malloc(sizeof(int) * init_cap);
    v->size = 0;
    v->capacity = init_cap;
}

static void vec_free(IntVec* v) {
    free(v->data);
    v->data = NULL;
    v->size = v->capacity = 0;
}

/* growth_factor: 10 = 1.0x, 15 = 1.5x, 20 = 2.0x (10분의 배율) */
static void vec_push_bench(IntVec* v, int val, int growth_factor_x10) {
    if (v->size == v->capacity) {
        size_t new_cap = v->capacity * (size_t)growth_factor_x10 / 10;
        if (new_cap <= v->capacity) new_cap = v->capacity + 1;
        int* tmp = realloc(v->data, sizeof(int) * new_cap);
        if (!tmp) { fprintf(stderr, "OOM\n"); exit(1); }
        v->data = tmp;
        v->capacity = new_cap;
    }
    v->data[v->size++] = val;
}

static double bench(int growth_factor_x10) {
    IntVec v;
    vec_init(&v, 4);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (size_t i = 0; i < PUSH_COUNT; i++) {
        vec_push_bench(&v, (int)i, growth_factor_x10);
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);

    double elapsed = (t1.tv_sec - t0.tv_sec) * 1000.0
                   + (t1.tv_nsec - t0.tv_nsec) / 1e6;

    printf("  확장 전략 x%.1f: %.1f ms, 최종 capacity=%zu, "
           "낭비=%.1f%%\n",
           growth_factor_x10 / 10.0,
           elapsed,
           v.capacity,
           (v.capacity - v.size) * 100.0 / v.capacity);

    vec_free(&v);
    return elapsed;
}

int main(void) {
    printf("%lu번 push 벤치마크:\n\n", PUSH_COUNT);

    bench(15);   /* 1.5배 */
    bench(20);   /* 2.0배 */
    bench(30);   /* 3.0배 (참고용) */

    printf("\n낭비율: 높을수록 메모리 비효율\n");
    printf("속도: 배율이 클수록 realloc 횟수가 적어 빠를 수 있음\n");

    return 0;
}
