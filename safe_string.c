// filename: safe_string.c
// gcc -std=c17 -O2 -D_FORTIFY_SOURCE=2 -o safe_string safe_string.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 안전한 입력 처리 */
char* safe_readline(FILE* fp, size_t max_len) {
    char* buf = malloc(max_len + 1);
    if (!buf) return NULL;

    if (!fgets(buf, (int)(max_len + 1), fp)) {
        free(buf);
        return NULL;
    }

    /* newline 제거 */
    buf[strcspn(buf, "\n\r")] = '\0';
    return buf;
}

/* ngx_str_t 스타일 슬라이스 */
typedef struct {
    const char* data;
    size_t      len;
} StrSlice;

StrSlice slice_create(const char* s, size_t start, size_t len) {
    return (StrSlice){ .data = s + start, .len = len };
}

void slice_print(StrSlice s) {
    printf("%.*s", (int)s.len, s.data);
}

int slice_eq(StrSlice a, StrSlice b) {
    return a.len == b.len && memcmp(a.data, b.data, a.len) == 0;
}

/* 안전한 URL 조합 */
int build_url(char* out, size_t out_size,
              const char* scheme, const char* host,
              int port, const char* path) {
    int n = snprintf(out, out_size, "%s://%s:%d%s",
                     scheme, host, port, path);
    if (n < 0 || (size_t)n >= out_size) {
        fprintf(stderr, "URL 조합 실패 (버퍼 부족: %d 필요, %zu 가용)\n",
                n, out_size);
        return -1;
    }
    return 0;
}

int main(void) {
    /* 안전한 URL 조합 */
    char url[128];
    if (build_url(url, sizeof(url), "https", "api.example.com", 443, "/v1/items") == 0) {
        printf("URL: %s\n", url);
    }

    /* 너무 긴 URL */
    char small_buf[20];
    build_url(small_buf, sizeof(small_buf), "https", "very.long.hostname.example.com", 8080, "/path");

    /* Nginx 스타일 슬라이스 */
    const char* request = "GET /index.html HTTP/1.1";
    /* "/index.html" 추출 (오프셋 4, 길이 12) */
    StrSlice path = slice_create(request, 4, 12);
    printf("경로: ");
    slice_print(path);
    printf("\n");

    /* 슬라이스 비교 */
    StrSlice html_ext = slice_create(".html", 0, 5);
    const char* ext_start = request + 4 + 7;   /* "/index" 이후 */
    StrSlice file_ext = { .data = ext_start, .len = 5 };
    printf("확장자 .html 여부: %s\n", slice_eq(html_ext, file_ext) ? "YES" : "NO");

    /* 안전한 입력 — stdin에서 읽기 */
    printf("\n이름 입력 (엔터): ");
    fflush(stdout);
    char* name = safe_readline(stdin, 63);
    if (name) {
        printf("안녕하세요, %s!\n", name);
        free(name);
    }

    return 0;
}
