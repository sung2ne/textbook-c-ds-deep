// filename: hash_chaining.c
// gcc -std=c17 -O2 -o hash_chaining hash_chaining.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ── FNV-1a 해시 (이전 챕터에서 가져옴) ── */
#define FNV_OFFSET_32  2166136261u
#define FNV_PRIME_32   16777619u

static uint32_t fnv1a(const char *key) {
    uint32_t hash = FNV_OFFSET_32;
    for (; *key; key++) {
        hash ^= (uint8_t)*key;
        hash *= FNV_PRIME_32;
    }
    return hash;
}

/* ── 연결 리스트 노드 ── */
typedef struct Entry {
    char         *key;
    int           value;
    struct Entry *next;
} Entry;

/* ── 해시 테이블 ── */
typedef struct {
    Entry  **buckets;
    int      bucket_count;
    int      size;
    float    load_factor_limit;
} HashMap;

/* ── 초기화 ── */
void hm_init(HashMap *hm, int bucket_count) {
    hm->bucket_count      = bucket_count;
    hm->size              = 0;
    hm->load_factor_limit = 0.75f;
    hm->buckets           = calloc(bucket_count, sizeof(Entry *));
}

/* ── 버킷 인덱스 계산 ── */
static int bucket_idx(const HashMap *hm, const char *key) {
    return (int)(fnv1a(key) % (uint32_t)hm->bucket_count);
}

/* ── 탐색 ── */
int *hm_get(HashMap *hm, const char *key) {
    int    idx  = bucket_idx(hm, key);
    Entry *cur  = hm->buckets[idx];
    while (cur) {
        if (strcmp(cur->key, key) == 0) return &cur->value;
        cur = cur->next;
    }
    return NULL;  /* not found */
}

/* ── rehashing ── */
static void hm_rehash(HashMap *hm) {
    int     new_count   = hm->bucket_count * 2;
    Entry **new_buckets = calloc(new_count, sizeof(Entry *));

    for (int i = 0; i < hm->bucket_count; i++) {
        Entry *cur = hm->buckets[i];
        while (cur) {
            Entry *next = cur->next;
            int    idx  = (int)(fnv1a(cur->key) % (uint32_t)new_count);
            cur->next         = new_buckets[idx];
            new_buckets[idx]  = cur;
            cur = next;
        }
    }
    free(hm->buckets);
    hm->buckets      = new_buckets;
    hm->bucket_count = new_count;
}

/* ── 삽입/업데이트 ── */
void hm_put(HashMap *hm, const char *key, int value) {
    /* 로드 팩터 초과 시 rehashing */
    if ((float)hm->size / hm->bucket_count > hm->load_factor_limit)
        hm_rehash(hm);

    int    idx = bucket_idx(hm, key);
    Entry *cur = hm->buckets[idx];

    /* 이미 존재하면 값만 업데이트 */
    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            cur->value = value;
            return;
        }
        cur = cur->next;
    }

    /* 새 항목 앞에 삽입 (head insertion) */
    Entry *entry   = malloc(sizeof(Entry));
    entry->key     = strdup(key);
    entry->value   = value;
    entry->next    = hm->buckets[idx];
    hm->buckets[idx] = entry;
    hm->size++;
}

/* ── 삭제 ── */
int hm_remove(HashMap *hm, const char *key) {
    int    idx  = bucket_idx(hm, key);
    Entry *cur  = hm->buckets[idx];
    Entry *prev = NULL;

    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            if (prev) prev->next       = cur->next;
            else      hm->buckets[idx] = cur->next;
            free(cur->key);
            free(cur);
            hm->size--;
            return 1;  /* 삭제 성공 */
        }
        prev = cur;
        cur  = cur->next;
    }
    return 0;  /* 키 없음 */
}

/* ── 해제 ── */
void hm_free(HashMap *hm) {
    for (int i = 0; i < hm->bucket_count; i++) {
        Entry *cur = hm->buckets[i];
        while (cur) {
            Entry *next = cur->next;
            free(cur->key);
            free(cur);
            cur = next;
        }
    }
    free(hm->buckets);
}

/* ── 통계 출력 ── */
void hm_stats(const HashMap *hm) {
    int max_chain = 0, empty = 0;
    for (int i = 0; i < hm->bucket_count; i++) {
        int len = 0;
        for (Entry *e = hm->buckets[i]; e; e = e->next) len++;
        if (len > max_chain) max_chain = len;
        if (len == 0) empty++;
    }
    printf("size=%d buckets=%d load=%.2f max_chain=%d empty=%d\n",
           hm->size, hm->bucket_count,
           (float)hm->size / hm->bucket_count,
           max_chain, empty);
}

int main(void) {
    HashMap hm;
    hm_init(&hm, 8);

    /* 삽입 */
    const char *names[] = {"alice", "bob", "carol", "dave",
                           "eve", "frank", "grace", "henry"};
    for (int i = 0; i < 8; i++)
        hm_put(&hm, names[i], i * 10);

    printf("삽입 후 통계: ");
    hm_stats(&hm);

    /* 탐색 */
    int *v = hm_get(&hm, "carol");
    printf("carol=%d\n", v ? *v : -1);

    /* 업데이트 */
    hm_put(&hm, "alice", 999);
    v = hm_get(&hm, "alice");
    printf("alice 업데이트 후=%d\n", v ? *v : -1);

    /* 삭제 */
    hm_remove(&hm, "bob");
    printf("bob 삭제 후: "); hm_stats(&hm);

    hm_free(&hm);
    return 0;
}
