// filename: hash_open_addressing.c
// gcc -std=c17 -O2 -o hash_open_addressing hash_open_addressing.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_SIZE   64
#define TOMBSTONE  ((char *)-1)   /* 삭제 표시 */

typedef enum { LINEAR, QUADRATIC, DOUBLE } ProbeMethod;

/* ── 슬롯 구조체 ── */
typedef struct {
    char *key;    /* NULL: 비어있음, TOMBSTONE: 삭제됨, 나머지: 유효 */
    int   value;
} Slot;

/* ── 해시 테이블 ── */
typedef struct {
    Slot        slots[MAX_SIZE];
    int         size;
    int         capacity;
    ProbeMethod method;
} OAHashMap;

/* ── 해시 함수 ── */
static uint32_t h1(const char *key) {
    uint32_t hash = 2166136261u;
    for (; *key; key++) { hash ^= (uint8_t)*key; hash *= 16777619u; }
    return hash;
}

/* Double Hashing 용 h2 — 소수 31 사용 */
static uint32_t h2(const char *key) {
    uint32_t hash = 0;
    for (; *key; key++) hash = hash * 31 + (uint8_t)*key;
    return (31 - (hash % 31)) | 1;  /* 홀수 보장 */
}

/* ── 탐사 인덱스 계산 ── */
static int probe(const OAHashMap *hm, const char *key, int i) {
    int cap = hm->capacity;
    uint32_t base = h1(key) % cap;
    switch (hm->method) {
        case LINEAR:    return (int)((base + i)        % cap);
        case QUADRATIC: return (int)((base + i * i)    % cap);
        case DOUBLE:    return (int)((base + i * h2(key)) % cap);
        default:        return (int)(base % cap);
    }
}

/* ── 초기화 ── */
void oa_init(OAHashMap *hm, ProbeMethod method) {
    memset(hm->slots, 0, sizeof(hm->slots));
    hm->size     = 0;
    hm->capacity = MAX_SIZE;
    hm->method   = method;
}

/* ── 탐색 ── */
int *oa_get(OAHashMap *hm, const char *key) {
    for (int i = 0; i < hm->capacity; i++) {
        int   idx  = probe(hm, key, i);
        Slot *slot = &hm->slots[idx];

        if (slot->key == NULL) return NULL;              /* 빈 슬롯 = 없음 */
        if (slot->key == TOMBSTONE) continue;            /* 삭제된 슬롯 건너뜀 */
        if (strcmp(slot->key, key) == 0) return &slot->value;
    }
    return NULL;
}

/* ── 삽입 ── */
bool oa_put(OAHashMap *hm, const char *key, int value) {
    if (hm->size >= hm->capacity * 2 / 3) return false;  /* 로드 팩터 2/3 초과 */

    for (int i = 0; i < hm->capacity; i++) {
        int   idx  = probe(hm, key, i);
        Slot *slot = &hm->slots[idx];

        if (slot->key == NULL || slot->key == TOMBSTONE) {
            slot->key   = strdup(key);
            slot->value = value;
            hm->size++;
            return true;
        }
        if (strcmp(slot->key, key) == 0) {
            slot->value = value;   /* 업데이트 */
            return true;
        }
    }
    return false;  /* 테이블 가득 참 */
}

/* ── 삭제 — tombstone 방식 ── */
bool oa_remove(OAHashMap *hm, const char *key) {
    for (int i = 0; i < hm->capacity; i++) {
        int   idx  = probe(hm, key, i);
        Slot *slot = &hm->slots[idx];

        if (slot->key == NULL) return false;
        if (slot->key == TOMBSTONE) continue;
        if (strcmp(slot->key, key) == 0) {
            free(slot->key);
            slot->key = TOMBSTONE;   /* 삭제 표시, 탐사 경로 보존 */
            hm->size--;
            return true;
        }
    }
    return false;
}

/* ── 탐사 횟수 통계 ── */
void oa_probe_stats(OAHashMap *hm) {
    long total = 0;
    int  max_p = 0, valid = 0;
    for (int slot_i = 0; slot_i < hm->capacity; slot_i++) {
        if (hm->slots[slot_i].key == NULL || hm->slots[slot_i].key == TOMBSTONE)
            continue;
        /* 이 키를 찾는 데 몇 번 탐사했는지 계산 */
        const char *key = hm->slots[slot_i].key;
        for (int i = 0; i < hm->capacity; i++) {
            int idx = probe(hm, key, i);
            if (hm->slots[idx].key && strcmp(hm->slots[idx].key, key) == 0) {
                total += (i + 1);
                if (i + 1 > max_p) max_p = i + 1;
                break;
            }
        }
        valid++;
    }
    const char *names[] = {"Linear", "Quadratic", "Double"};
    printf("[%s] avg_probe=%.2f max_probe=%d\n",
           names[hm->method],
           valid ? (double)total / valid : 0.0, max_p);
}

void oa_free(OAHashMap *hm) {
    for (int i = 0; i < hm->capacity; i++)
        if (hm->slots[i].key && hm->slots[i].key != TOMBSTONE)
            free(hm->slots[i].key);
}

int main(void) {
    const char *keys[] = {
        "alpha", "beta", "gamma", "delta", "epsilon",
        "zeta",  "eta",  "theta", "iota",  "kappa",
        "lambda","mu",   "nu",    "xi",    "omicron",
        "pi",    "rho",  "sigma", "tau",   "upsilon"
    };
    int n = 20;

    ProbeMethod methods[] = {LINEAR, QUADRATIC, DOUBLE};
    for (int m = 0; m < 3; m++) {
        OAHashMap hm;
        oa_init(&hm, methods[m]);
        for (int i = 0; i < n; i++) oa_put(&hm, keys[i], i);
        oa_probe_stats(&hm);
        oa_free(&hm);
    }

    /* 삭제·tombstone 동작 확인 */
    OAHashMap hm;
    oa_init(&hm, LINEAR);
    oa_put(&hm, "foo", 1);
    oa_put(&hm, "bar", 2);
    oa_remove(&hm, "foo");
    printf("foo 삭제 후 bar 탐색: %s\n",
           oa_get(&hm, "bar") ? "성공" : "실패");
    oa_free(&hm);
    return 0;
}
