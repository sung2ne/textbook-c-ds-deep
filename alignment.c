// filename: alignment.c
// gcc -std=c17 -O2 -o alignment alignment.c

#include <stdio.h>
#include <stdint.h>
#include <stdalign.h>
#include <string.h>

/* 패딩이 많은 구조체 */
struct Bad {
    char  a;
    int   b;
    char  c;
    double d;
};

/* 패딩이 적은 구조체 (큰 타입 먼저) */
struct Good {
    double d;
    int    b;
    char   a;
    char   c;
    /* 2바이트 패딩 */
};

/* packed 구조체 — 네트워크 패킷 파싱용 */
#pragma pack(1)
struct PackedHeader {
    uint8_t  version;
    uint16_t length;
    uint32_t sequence;
    uint8_t  flags;
};
#pragma pack()

/* 캐시 라인 정렬 */
alignas(64) static int cache_line_data[16];

int main(void) {
    /* 구조체 크기 비교 */
    printf("struct Bad  크기: %zu바이트 (기대: %zu)\n",
           sizeof(struct Bad),
           sizeof(char) + sizeof(int) + sizeof(char) + sizeof(double));
    printf("struct Good 크기: %zu바이트\n", sizeof(struct Good));
    printf("PackedHeader 크기: %zu바이트 (기대: 8)\n", sizeof(struct PackedHeader));

    /* 멤버 오프셋 확인 */
    struct Bad b;
    printf("\nstruct Bad 멤버 오프셋:\n");
    printf("  a: %zu\n", (size_t)((char*)&b.a - (char*)&b));
    printf("  b: %zu\n", (size_t)((char*)&b.b - (char*)&b));
    printf("  c: %zu\n", (size_t)((char*)&b.c - (char*)&b));
    printf("  d: %zu\n", (size_t)((char*)&b.d - (char*)&b));

    /* 정렬 요구사항 */
    printf("\n기본 타입 정렬:\n");
    printf("  char:   %zu\n", alignof(char));
    printf("  int:    %zu\n", alignof(int));
    printf("  double: %zu\n", alignof(double));

    /* 캐시 라인 정렬 확인 */
    printf("\ncache_line_data 주소: %p\n", (void*)cache_line_data);
    printf("64 정렬 여부: %s\n",
           ((uintptr_t)cache_line_data % 64 == 0) ? "OK" : "FAIL");

    /* packed 구조체로 바이트 스트림 파싱 */
    uint8_t raw_bytes[] = {0x01, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x00};
    struct PackedHeader hdr;
    memcpy(&hdr, raw_bytes, sizeof(hdr));
    printf("\nPacked 헤더 파싱:\n");
    printf("  version: %u\n", hdr.version);
    printf("  length:  %u\n", hdr.length);
    printf("  sequence: %u\n", hdr.sequence);
    printf("  flags:   %u\n", hdr.flags);

    return 0;
}
