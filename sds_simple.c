// filename: sds_simple.c
// gcc -std=c17 -O2 -o sds_simple sds_simple.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* 간략화된 sds 구현 (학습용) */

#define SDS_HDR_SIZE 8   /* len(4) + cap(4) */

typedef char* sds_t;

typedef struct {
    uint32_t len;
    uint32_t cap;
    char     buf[];   /* 유연 배열 멤버 */
} SdsHdr;

/* 헤더 접근 매크로 */
#define SDS_HDR(s)  ((SdsHdr*)((s) - sizeof(SdsHdr)))
#define SDS_LEN(s)  (SDS_HDR(s)->len)
#define SDS_CAP(s)  (SDS_HDR(s)->cap)

sds_t sds_new(const char* init) {
    size_t init_len = init ? strlen(init) : 0;
    size_t alloc_size = sizeof(SdsHdr) + init_len + 1;   /* +1: NULL */

    SdsHdr* hdr = malloc(alloc_size);
    if (!hdr) return NULL;

    hdr->len = (uint32_t)init_len;
    hdr->cap = (uint32_t)init_len;

    if (init_len > 0) memcpy(hdr->buf, init, init_len);
    hdr->buf[init_len] = '\0';   /* NULL 종료 — C 함수 호환 */

    return hdr->buf;   /* 데이터 시작점 반환 */
}

sds_t sds_cat(sds_t s, const char* t) {
    size_t t_len = strlen(t);
    size_t cur_len = SDS_LEN(s);
    size_t cur_cap = SDS_CAP(s);
    size_t need = cur_len + t_len;

    if (need > cur_cap) {
        /* 확장: 필요량의 2배 */
        size_t new_cap = need * 2;
        SdsHdr* hdr = realloc(SDS_HDR(s), sizeof(SdsHdr) + new_cap + 1);
        if (!hdr) return NULL;
        hdr->cap = (uint32_t)new_cap;
        s = hdr->buf;
    }

    memcpy(s + cur_len, t, t_len + 1);   /* +1: NULL 포함 복사 */
    SDS_HDR(s)->len = (uint32_t)need;
    return s;
}

void sds_free(sds_t s) {
    if (s) free(SDS_HDR(s));
}

size_t sds_len(const sds_t s) { return SDS_LEN(s); }
size_t sds_avail(const sds_t s) { return SDS_CAP(s) - SDS_LEN(s); }

int main(void) {
    /* 기본 생성 */
    sds_t s = sds_new("Hello");
    printf("문자열: \"%s\"\n", s);
    printf("길이: %zu (O(1)!)\n", sds_len(s));

    /* 이어붙이기 */
    s = sds_cat(s, ", World");
    s = sds_cat(s, "!");
    printf("이어붙인 후: \"%s\"\n", s);
    printf("길이: %zu, 여유: %zu\n", sds_len(s), sds_avail(s));

    /* 이진 안전: \0 이후에도 데이터 저장 가능 */
    SdsHdr* hdr = SDS_HDR(s);
    printf("\n헤더 정보: len=%u, cap=%u\n", hdr->len, hdr->cap);

    /* printf와 호환 */
    printf("printf 호환: %s\n", s);

    sds_free(s);
    return 0;
}
