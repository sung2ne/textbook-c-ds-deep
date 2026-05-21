// filename: string_basics.c
// gcc -std=c17 -O2 -o string_basics string_basics.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 안전한 문자열 복사 (NULL 종료 보장) */
void safe_strcpy(char* dst, size_t dst_size, const char* src) {
    snprintf(dst, dst_size, "%s", src);
}

/* 안전한 문자열 이어붙이기 */
void safe_strcat(char* dst, size_t dst_size, const char* src) {
    size_t len = strlen(dst);
    if (len >= dst_size) return;   /* 이미 가득 찼음 */
    snprintf(dst + len, dst_size - len, "%s", src);
}

/* 문자열 뒤집기 (in-place) */
void str_reverse(char* s) {
    size_t len = strlen(s);
    for (size_t lo = 0, hi = len - 1; lo < hi; lo++, hi--) {
        char tmp = s[lo]; s[lo] = s[hi]; s[hi] = tmp;
    }
}

/* 부분 문자열 포함 여부 */
int str_contains(const char* haystack, const char* needle) {
    return strstr(haystack, needle) != NULL;
}

int main(void) {
    /* strlen 주의 */
    char s[] = "Hello, World!";
    printf("strlen: %zu, sizeof: %zu\n", strlen(s), sizeof(s));

    /* 안전한 복사 */
    char buf[10];
    safe_strcpy(buf, sizeof(buf), s);
    printf("safe_strcpy 결과: \"%s\" (잘렸는가: %s)\n",
           buf, (strlen(buf) < strlen(s)) ? "YES" : "NO");

    /* 안전한 이어붙이기 */
    char greeting[20];
    safe_strcpy(greeting, sizeof(greeting), "Hello");
    safe_strcat(greeting, sizeof(greeting), ", World");
    safe_strcat(greeting, sizeof(greeting), "!");
    printf("이어붙이기: \"%s\"\n", greeting);

    /* strcmp */
    const char* a = "apple";
    const char* b = "banana";
    const char* c = "apple";
    printf("\nstrcmp:\n");
    printf("  \"%s\" vs \"%s\": %d\n", a, b, strcmp(a, b));   /* 음수 */
    printf("  \"%s\" vs \"%s\": %d\n", b, a, strcmp(b, a));   /* 양수 */
    printf("  \"%s\" == \"%s\": %s\n", a, c,
           (strcmp(a, c) == 0) ? "YES" : "NO");   /* YES */
    /* 포인터 비교 (틀린 방법) */
    printf("  포인터 비교 ==: %s (틀린 방법)\n",
           (a == c) ? "같음" : "다름");   /* "다름" — 다른 주소 */

    /* 뒤집기 */
    char rev[] = "abcdef";
    str_reverse(rev);
    printf("\n역순: %s\n", rev);

    /* strstr */
    printf("포함 여부: \"%s\" in \"%s\": %s\n",
           "World", s,
           str_contains(s, "World") ? "YES" : "NO");

    /* snprintf 반환값으로 잘림 감지 */
    char small[5];
    int n = snprintf(small, sizeof(small), "%s", "LongString");
    if (n >= (int)sizeof(small)) {
        printf("\n경고: 잘림 발생 (필요: %d바이트, 가용: %zu바이트)\n",
               n, sizeof(small));
    }

    return 0;
}
