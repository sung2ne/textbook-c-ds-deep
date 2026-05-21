# 깊이 파는 C 자료구조 — 실습 코드

> [교재 바로가기](https://text.ibetter.kr/c-ds-deep)

이 저장소는 **깊이 파는 C 자료구조** 교재의 챕터별 실습 코드를 담고 있습니다.

## 기술 스택

- **언어**: C17 / C23
- **컴파일러**: GCC 14+ / Apple Clang 17+
- **표준**: ISO C17, POSIX
- **빌드**: `gcc -std=c17 -O2 -Wall -Wextra`

## 브랜치 구조

각 챕터의 코드는 별도 브랜치로 관리됩니다.

```
part01/chapter-01   ← PART 01 / Ch 01까지의 누적 코드
part01/chapter-02
...
part12/chapter-07   ← 최종 챕터
main                ← 모든 챕터 코드 누적 (최신)
```

## 컴파일 방법

대부분의 파일은 단독으로 컴파일됩니다.

```bash
gcc -std=c17 -O2 -Wall -o output filename.c
```

일부 파일은 특수한 플래그가 필요합니다.

```bash
# pthread 사용
gcc -std=c17 -O2 -pthread -o output filename.c

# AddressSanitizer
gcc -std=c17 -fsanitize=address -g -o output filename.c
```

각 파일 상단의 `// gcc ...` 주석을 참고하세요.

## 교재 구성

| PART | 주제 |
|------|------|
| PART 01 | 자료구조를 보는 눈 — 복잡도와 측정 |
| PART 02 | 메모리와 포인터 — C의 심장 |
| PART 03 | 배열과 문자열 |
| PART 04 | 연결 자료구조 직접 구현 |
| PART 05 | 정렬 알고리즘 |
| PART 06 | 트리와 힙 |
| PART 07 | 해시 테이블 직접 구현 |
| PART 08 | 그래프 알고리즘 |
| PART 09 | ★심화 선택 — 메모리 안전 도구 |
| PART 10 | ★심화 선택 — 캐시 친화적 자료구조 |
| PART 11 | ★심화 선택 — Lock-Free 자료구조 |
| PART 12 | ★심화 선택 — glibc 내부와 Custom Allocator |
