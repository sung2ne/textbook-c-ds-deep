// filename: hash_functions.c
// gcc -std=c17 -O2 -o hash_functions hash_functions.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ── FNV-1a 32비트 ── */
#define FNV_OFFSET_32  2166136261u
#define FNV_PRIME_32   16777619u

uint32_t fnv1a_32(const void *data, size_t len) {
    uint32_t    hash = FNV_OFFSET_32;
    const uint8_t *p = (const uint8_t *)data;
    for (size_t i = 0; i < len; i++) {
        hash ^= p[i];
        hash *= FNV_PRIME_32;
    }
    return hash;
}

/* ── FNV-1a 64비트 ── */
#define FNV_OFFSET_64  14695981039346656037ULL
#define FNV_PRIME_64   1099511628211ULL

uint64_t fnv1a_64(const void *data, size_t len) {
    uint64_t    hash = FNV_OFFSET_64;
    const uint8_t *p = (const uint8_t *)data;
    for (size_t i = 0; i < len; i++) {
        hash ^= p[i];
        hash *= FNV_PRIME_64;
    }
    return hash;
}
