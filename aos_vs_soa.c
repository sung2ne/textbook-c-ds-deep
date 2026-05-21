// filename: aos_vs_soa.c
// gcc -std=c17 -O3 -march=native -o aos_vs_soa aos_vs_soa.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define N 1000000   /* 100만 엔티티 */

/* ── AoS 레이아웃 ──────────────────────────────── */
typedef struct {
    float x, y, z;   /* 위치 */
    float vx, vy, vz; /* 속도 */
} EntityAoS;

EntityAoS aos[N];

void update_aos(float dt) {
    for (int i = 0; i < N; i++) {
        aos[i].x += aos[i].vx * dt;
        aos[i].y += aos[i].vy * dt;
        aos[i].z += aos[i].vz * dt;
    }
}

/* ── SoA 레이아웃 ──────────────────────────────── */
float soa_x[N], soa_y[N], soa_z[N];
float soa_vx[N], soa_vy[N], soa_vz[N];

void update_soa(float dt) {
    for (int i = 0; i < N; i++) {
        soa_x[i] += soa_vx[i] * dt;
        soa_y[i] += soa_vy[i] * dt;
        soa_z[i] += soa_vz[i] * dt;
    }
}

static inline uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int main(void) {
    /* 초기화 */
    for (int i = 0; i < N; i++) {
        aos[i] = (EntityAoS){1.0f, 2.0f, 3.0f, 0.1f, 0.2f, 0.3f};
        soa_x[i] = 1.0f; soa_y[i] = 2.0f; soa_z[i] = 3.0f;
        soa_vx[i] = 0.1f; soa_vy[i] = 0.2f; soa_vz[i] = 0.3f;
    }

    int runs = 100;
    float dt = 0.016f;

    uint64_t t0 = now_ns();
    for (int r = 0; r < runs; r++) update_aos(dt);
    uint64_t t1 = now_ns();

    uint64_t t2 = now_ns();
    for (int r = 0; r < runs; r++) update_soa(dt);
    uint64_t t3 = now_ns();

    printf("AoS: %.2f ms/run\n", (t1 - t0) / 1e6 / runs);
    printf("SoA: %.2f ms/run\n", (t3 - t2) / 1e6 / runs);
    printf("속도비: %.1fx\n", (double)(t1 - t0) / (t3 - t2));
    return 0;
}
