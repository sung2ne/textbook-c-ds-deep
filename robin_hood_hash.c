// filename: robin_hood_hash.c
// gcc -std=c17 -O2 -o robin_hood_hash robin_hood_hash.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define RH_CAPACITY 32

/* ── FNV-1a ── */
static uint32_t fnv1a(const char *key) {
    uint32_t h = 2166136261u;
    for (; *key; key++) { h ^= (uint8_t)*key; h *= 16777619u; }
    return h;
}

/* ── 슬롯 ── */
typedef struct {
    char *key;     /* NULL = empty */
    int   value;
    int   dib;     /* Distance from Initial Bucket */
} RHSlot;

/* ── Robin Hood 해시 테이블 ── */
typedef struct {
    RHSlot slots[RH_CAPACITY];
    int    size;
    int    capacity;
} RHHashMap;

void rh_init(RHHashMap *hm) {
    memset(hm->slots, 0, sizeof(hm->slots));
    hm->size     = 0;
    hm->capacity = RH_CAPACITY;
}

static int ideal_idx(const RHHashMap *hm, const char *key) {
    return (int)(fnv1a(key) % (uint32_t)hm->capacity);
}

/* ── 탐색 ── */
int *rh_get(RHHashMap *hm, const char *key) {
    int ideal = ideal_idx(hm, key);
    for (int i = 0; i < hm->capacity; i++) {
        int    idx  = (ideal + i) % hm->capacity;
        RHSlot *s   = &hm->slots[idx];
        if (!s->key) return NULL;            /* 빈 슬롯 = 없음 */
        if (s->dib < i) return NULL;         /* 현재 DIB < 탐사 횟수 → 없음 */
        if (strcmp(s->key, key) == 0) return &s->value;
    }
    return NULL;
}

/* ── 삽입 (Robin Hood: 빈자가 부자 자리 빼앗기) ── */
bool rh_put(RHHashMap *hm, const char *key, int value) {
    if (hm->size >= hm->capacity * 3 / 4) return false;

    /* 삽입할 항목을 임시 변수에 */
    char   *ins_key   = strdup(key);
    int     ins_val   = value;
    int     ins_dib   = 0;
    int     ideal     = ideal_idx(hm, key);

    for (int i = 0; i < hm->capacity; i++) {
        int    idx = (ideal + i) % hm->capacity;
        RHSlot *s  = &hm->slots[idx];

        if (!s->key) {
            /* 빈 슬롯 — 배치 */
            s->key   = ins_key;
            s->value = ins_val;
            s->dib   = ins_dib;
            hm->size++;
            return true;
        }

        /* 기존 키와 동일 — 업데이트 */
        if (strcmp(s->key, key) == 0) {
            s->value = ins_val;
            free(ins_key);
            return true;
        }

        /* Robin Hood: 삽입 중인 키의 DIB가 더 크면 자리 교환 */
        if (ins_dib > s->dib) {
            /* 현재 슬롯 내용을 ins로 교환 */
            char *tmp_key = s->key;   int tmp_val = s->value; int tmp_dib = s->dib;
            s->key   = ins_key;  s->value = ins_val;  s->dib = ins_dib;
            ins_key  = tmp_key;  ins_val  = tmp_val;  ins_dib = tmp_dib;
        }

        ins_dib++;
    }
    free(ins_key);
    return false;
}

/* ── 삭제 (Backward Shift Deletion — tombstone 없음) ── */
bool rh_remove(RHHashMap *hm, const char *key) {
    int ideal = ideal_idx(hm, key);
    int found = -1;

    for (int i = 0; i < hm->capacity; i++) {
        int    idx = (ideal + i) % hm->capacity;
        RHSlot *s  = &hm->slots[idx];
        if (!s->key || s->dib < i) return false;
        if (strcmp(s->key, key) == 0) { found = idx; break; }
    }
    if (found < 0) return false;

    free(hm->slots[found].key);
    hm->slots[found].key = NULL;
    hm->size--;

    /* Backward shift: 뒤의 키들을 앞으로 당기기 */
    for (int i = 1; i < hm->capacity; i++) {
        int cur  = (found + i) % hm->capacity;
        int prev = (found + i - 1) % hm->capacity;
        RHSlot *s = &hm->slots[cur];
        if (!s->key || s->dib == 0) break;   /* 원래 자리에 있는 키면 멈춤 */
        hm->slots[prev]   = *s;
        hm->slots[prev].dib--;
        s->key = NULL;
    }
    return true;
}

/* ── DIB 통계 ── */
void rh_stats(const RHHashMap *hm) {
    double sum = 0; int max_dib = 0, cnt = 0;
    for (int i = 0; i < hm->capacity; i++) {
        if (!hm->slots[i].key) continue;
        sum += hm->slots[i].dib;
        if (hm->slots[i].dib > max_dib) max_dib = hm->slots[i].dib;
        cnt++;
    }
    printf("size=%d avg_dib=%.2f max_dib=%d\n",
           hm->size, cnt ? sum / cnt : 0.0, max_dib);
}

void rh_free(RHHashMap *hm) {
    for (int i = 0; i < hm->capacity; i++)
        if (hm->slots[i].key) free(hm->slots[i].key);
}

int main(void) {
    RHHashMap hm;
    rh_init(&hm);

    const char *keys[] = {
        "redis", "kafka", "nginx", "postgres", "mysql",
        "sqlite", "mongodb", "elasticsearch", "cassandra", "influxdb",
        "rabbitmq", "memcached", "consul", "etcd", "vault"
    };
    for (int i = 0; i < 15; i++)
        rh_put(&hm, keys[i], i);

    printf("삽입 후 통계: ");
    rh_stats(&hm);

    rh_remove(&hm, "nginx");
    rh_remove(&hm, "kafka");
    printf("2개 삭제 후: ");
    rh_stats(&hm);

    printf("redis 탐색: %s\n", rh_get(&hm, "redis") ? "있음" : "없음");
    printf("kafka 탐색: %s\n", rh_get(&hm, "kafka") ? "있음" : "없음");

    rh_free(&hm);
    return 0;
}
