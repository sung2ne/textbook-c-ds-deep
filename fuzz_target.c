// filename: fuzz_target.c
// clang -fsanitize=fuzzer,address -g -O1 -o fuzz_target fuzz_target.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ───────── fuzzing할 파서 함수 ───────── */
/*
 * 간단한 TLV(Type-Length-Value) 파서.
 * 실제 네트워크 프로토콜, 파일 포맷 파서가 이런 구조를 가집니다.
 */
int parse_tlv(const uint8_t *data, size_t size) {
    if (size < 2) return -1;

    uint8_t type   = data[0];
    uint8_t length = data[1];

    if (size < (size_t)(2 + length)) return -1;

    char value[256];
    /* 버그: length가 255보다 크면 버퍼 오버플로우! */
    memcpy(value, data + 2, length);
    value[length] = '\0';

    if (type == 0x01) {
        printf("이름: %s\n", value);
    } else if (type == 0x02) {
        printf("나이: %s\n", value);
    }
    return 0;
}

/* ───────── libFuzzer 진입점 ───────── */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    parse_tlv(data, size);
    return 0;  /* 0: 정상, -1: 이 입력 버리기 */
}

/*
=== 빌드 및 실행 ===

clang -fsanitize=fuzzer,address -g -O1 -o fuzz_target fuzz_target.c

# fuzzing 시작 (corpus 폴더에 seed 입력 가능)
mkdir -p corpus
./fuzz_target corpus/ -max_total_time=60

# 충돌 발생 시 자동으로 crash-<hash> 파일 생성
# 재현: ./fuzz_target crash-<hash>
*/
