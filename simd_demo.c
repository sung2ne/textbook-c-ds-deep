// filename: simd_demo.c
// gcc -std=c17 -O2 -mavx2 -o simd_demo simd_demo.c
// Apple M 시리즈: clang -std=c17 -O2 -o simd_demo simd_demo.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* ── x86 AVX2 ─────────────────────────────────── */
#if defined(__AVX2__)
#include <immintrin.h>

/* a[i] = a[i] + b[i] * scalar, AVX2 버전 (8개씩 처리) */
void add_scaled_avx(float * restrict a, const float * restrict b,
                    float scalar, int n) {
    __m256 vscalar = _mm256_set1_ps(scalar);  /* scalar를 8개 복제 */
    int i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256 va = _mm256_loadu_ps(&a[i]);   /* a[i..i+7] 로드 */
        __m256 vb = _mm256_loadu_ps(&b[i]);   /* b[i..i+7] 로드 */
        __m256 vr = _mm256_fmadd_ps(vb, vscalar, va); /* va + vb*vscalar */
        _mm256_storeu_ps(&a[i], vr);           /* 결과 저장 */
    }
    /* 나머지 처리 (n이 8의 배수가 아닐 때) */
    for (; i < n; i++) a[i] += b[i] * scalar;
}

/* 스칼라 버전 (비교용) */
void add_scaled_scalar(float * restrict a, const float * restrict b,
                       float scalar, int n) {
    for (int i = 0; i < n; i++) a[i] += b[i] * scalar;
}

#elif defined(__ARM_NEON)
/* ── ARM NEON ──────────────────────────────────── */
#include <arm_neon.h>

void add_scaled_avx(float * restrict a, const float * restrict b,
                    float scalar, int n) {
    float32x4_t vscalar = vdupq_n_f32(scalar);
    int i = 0;
    for (; i + 4 <= n; i += 4) {
        float32x4_t va = vld1q_f32(&a[i]);
        float32x4_t vb = vld1q_f32(&b[i]);
        float32x4_t vr = vmlaq_f32(va, vb, vscalar); /* va + vb*vscalar */
        vst1q_f32(&a[i], vr);
    }
    for (; i < n; i++) a[i] += b[i] * scalar;
}

void add_scaled_scalar(float * restrict a, const float * restrict b,
                       float scalar, int n) {
    for (int i = 0; i < n; i++) a[i] += b[i] * scalar;
}

#else
/* ── fallback ─────────────────────────────────── */
void add_scaled_avx(float * restrict a, const float * restrict b,
                    float scalar, int n) {
    for (int i = 0; i < n; i++) a[i] += b[i] * scalar;
}
void add_scaled_scalar(float * restrict a, const float * restrict b,
                       float scalar, int n) {
    for (int i = 0; i < n; i++) a[i] += b[i] * scalar;
}
#endif

#define N (1 << 23)   /* 8M floats = 32MB */

static float a1[N], b1[N];
static float a2[N], b2[N];

static inline uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int main(void) {
    for (int i = 0; i < N; i++) {
        a1[i] = a2[i] = (float)i * 0.001f;
        b1[i] = b2[i] = (float)(N - i) * 0.001f;
    }

    int runs = 20;
    float scalar = 0.016f;

    uint64_t t0 = now_ns();
    for (int r = 0; r < runs; r++) add_scaled_scalar(a1, b1, scalar, N);
    uint64_t t1 = now_ns();

    uint64_t t2 = now_ns();
    for (int r = 0; r < runs; r++) add_scaled_avx(a2, b2, scalar, N);
    uint64_t t3 = now_ns();

    printf("스칼라: %.2f ms/run\n", (t1 - t0) / 1e6 / runs);
    printf("SIMD  : %.2f ms/run\n", (t3 - t2) / 1e6 / runs);
    printf("속도비: %.1fx\n", (double)(t1 - t0) / (double)(t3 - t2));
    return 0;
}
