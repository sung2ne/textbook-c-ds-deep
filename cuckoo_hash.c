// filename: cuckoo_hash.c
// gcc -std=c17 -O2 -o cuckoo_hash cuckoo_hash.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define CK_SIZE  32       /* 각 테이블 크기 (두 개의 테이블 사용) */
#define MAX_LOOP 64       /* 삽입 시 최대 쫓아내기 횟수 */

/* ── 두 해시 함수 ── */
static uint32_t h1(const char *key) {
    uint32_t h = 2166136261u;
    for (; *key; key++) { h ^= (uint8_t)*key; h *= 16777619u; }
    return h % CK_SIZE;
}

static uint32_t h2(const char *key) {
    uint32_t h = 0;
    for (; *key; key++) h = h * 2654435761u ^ (uint8_t)*key;
    return h % CK_SIZE;
}

/* ── 슬롯 ── */
typedef struct {
    char *key;
    int   value;
} CKSlot;

/* ── Cuckoo 해시 테이블 — 두 개의 테이블 ── */
typedef struct {
    CKSlot table1[CK_SIZE];   /* h1 기반 */
    CKSlot table2[CK_SIZE];   /* h2 기반 */
    int    size;
} CuckooMap;

void ck_init(CuckooMap *cm) {
    memset(cm, 0, sizeof(*cm));
}

/* ── 탐색: 최대 2번만 확인 — Worst Case O(1) ── */
int *ck_get(CuckooMap *cm, const char *key) {
    uint32_t i1 = h1(key);
    if (cm->table1[i1].key && strcmp(cm->table1[i1].key, key) == 0)
        return &cm->table1[i1].value;

    uint32_t i2 = h2(key);
    if (cm->table2[i2].key && strcmp(cm->table2[i2].key, key) == 0)
        return &cm->table2[i2].value;

    return NULL;
}

/* ── 삽입 (cuckoo eviction) ── */
static bool ck_insert_entry(CuckooMap *cm, char *key, int value);

bool ck_put(CuckooMap *cm, const char *key, int value) {
    /* 이미 존재하면 업데이트 */
    int *v = ck_get(cm, key);
    if (v) { *v = value; return true; }
    return ck_insert_entry(cm, strdup(key), value);
}

static bool ck_insert_entry(CuckooMap *cm, char *key, int value) {
    for (int loop = 0; loop < MAX_LOOP; loop++) {
        uint32_t i1 = h1(key);

        if (!cm->table1[i1].key) {
            /* table1 빈 슬롯 — 삽입 완료 */
            cm->table1[i1].key   = key;
            cm->table1[i1].value = value;
            cm->size++;
            return true;
        }

        /* table1 충돌 — 기존 키를 쫓아내고 자리 차지 */
        char *evicted_key   = cm->table1[i1].key;
        int   evicted_val   = cm->table1[i1].value;
        cm->table1[i1].key   = key;
        cm->table1[i1].value = value;
        key   = evicted_key;
        value = evicted_val;

        /* 쫓겨난 키를 table2에 삽입 시도 */
        uint32_t i2 = h2(key);
        if (!cm->table2[i2].key) {
            cm->table2[i2].key   = key;
            cm->table2[i2].value = value;
            cm->size++;
            return true;
        }

        /* table2도 충돌 — 다시 쫓아냄 */
        evicted_key = cm->table2[i2].key;
        evicted_val = cm->table2[i2].value;
        cm->table2[i2].key   = key;
        cm->table2[i2].value = value;
        key   = evicted_key;
        value = evicted_val;
        /* 다음 loop에서 key를 table1에 다시 삽입 시도 */
    }

    /* MAX_LOOP 초과 — 사이클 감지, rehashing 필요 */
    free(key);
    return false;
}

/* ── 삭제 ── */
bool ck_remove(CuckooMap *cm, const char *key) {
    uint32_t i1 = h1(key);
    if (cm->table1[i1].key && strcmp(cm->table1[i1].key, key) == 0) {
        free(cm->table1[i1].key);
        cm->table1[i1].key = NULL;
        cm->size--;
        return true;
    }
    uint32_t i2 = h2(key);
    if (cm->table2[i2].key && strcmp(cm->table2[i2].key, key) == 0) {
        free(cm->table2[i2].key);
        cm->table2[i2].key = NULL;
        cm->size--;
        return true;
    }
    return false;
}

void ck_free(CuckooMap *cm) {
    for (int i = 0; i < CK_SIZE; i++) {
        if (cm->table1[i].key) free(cm->table1[i].key);
        if (cm->table2[i].key) free(cm->table2[i].key);
    }
}

int main(void) {
    CuckooMap cm;
    ck_init(&cm);

    const char *keys[] = {
        "one", "two", "three", "four", "five",
        "six", "seven", "eight", "nine", "ten"
    };

    for (int i = 0; i < 10; i++) {
        bool ok = ck_put(&cm, keys[i], i + 1);
        printf("삽입 %-8s : %s\n", keys[i], ok ? "성공" : "실패(사이클)");
    }

    printf("\n탐색:\n");
    for (int i = 0; i < 10; i++) {
        int *v = ck_get(&cm, keys[i]);
        printf("  %-8s → %d\n", keys[i], v ? *v : -1);
    }

    ck_remove(&cm, "five");
    printf("\nfive 삭제 후: %s\n",
           ck_get(&cm, "five") ? "있음" : "없음");

    printf("size=%d\n", cm.size);
    ck_free(&cm);
    return 0;
}
