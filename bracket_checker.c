// filename: bracket_checker.c
// gcc -std=c17 -O2 -o bracket_checker bracket_checker.c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* 괄호 위치 정보 */
typedef struct {
    char   ch;    /* 여는 괄호 문자 */
    int    line;  /* 줄 번호 (1-based) */
    int    col;   /* 열 번호 (1-based) */
} BracketInfo;

/* 배열 기반 스택 (BracketInfo 전용) */
typedef struct {
    BracketInfo *data;
    int          top;
    int          capacity;
} BracketStack;

static void bs_init(BracketStack *s, int capacity) {
    s->data = malloc(sizeof(BracketInfo) * (size_t)capacity);
    if (!s->data) { perror("malloc"); exit(1); }
    s->top = -1;
    s->capacity = capacity;
}

static void bs_push(BracketStack *s, BracketInfo bi) {
    if (s->top >= s->capacity - 1) {
        s->capacity *= 2;
        s->data = realloc(s->data, sizeof(BracketInfo) * (size_t)s->capacity);
    }
    s->data[++s->top] = bi;
}

static bool bs_pop(BracketStack *s, BracketInfo *out) {
    if (s->top < 0) return false;
    *out = s->data[s->top--];
    return true;
}

static bool bs_empty(const BracketStack *s) { return s->top < 0; }
static void bs_destroy(BracketStack *s) { free(s->data); }

/* 괄호 종류 확인 */
static bool is_open(char c)  { return c == '(' || c == '[' || c == '{'; }
static bool is_close(char c) { return c == ')' || c == ']' || c == '}'; }

static char matching_open(char close) {
    switch (close) {
        case ')': return '(';
        case ']': return '[';
        case '}': return '{';
        default:  return '\0';
    }
}

/* 검사 결과 */
typedef struct {
    bool ok;
    int  error_line;
    int  error_col;
    char error_msg[128];
} CheckResult;

CheckResult check_brackets(const char *source) {
    CheckResult result = {true, 0, 0, ""};
    BracketStack stack;
    bs_init(&stack, 64);

    int line = 1, col = 1;
    for (const char *p = source; *p; p++, col++) {
        if (*p == '\n') { line++; col = 0; continue; }

        if (is_open(*p)) {
            BracketInfo bi = {*p, line, col};
            bs_push(&stack, bi);
        } else if (is_close(*p)) {
            BracketInfo top;
            if (!bs_pop(&stack, &top)) {
                result.ok = false;
                result.error_line = line;
                result.error_col  = col;
                snprintf(result.error_msg, sizeof(result.error_msg),
                         "닫는 괄호 '%c'에 대응하는 여는 괄호 없음", *p);
                goto cleanup;
            }
            if (top.ch != matching_open(*p)) {
                result.ok = false;
                result.error_line = line;
                result.error_col  = col;
                snprintf(result.error_msg, sizeof(result.error_msg),
                         "'%c' (줄 %d, 열 %d)에 열린 괄호가 '%c' (줄 %d, 열 %d)에서 닫힘",
                         top.ch, top.line, top.col, *p, line, col);
                goto cleanup;
            }
        }
    }

    if (!bs_empty(&stack)) {
        BracketInfo unclosed;
        bs_pop(&stack, &unclosed);
        result.ok = false;
        result.error_line = unclosed.line;
        result.error_col  = unclosed.col;
        snprintf(result.error_msg, sizeof(result.error_msg),
                 "'%c' (줄 %d, 열 %d)에 열린 괄호가 닫히지 않음",
                 unclosed.ch, unclosed.line, unclosed.col);
    }

cleanup:
    bs_destroy(&stack);
    return result;
}

static void test_case(const char *desc, const char *source) {
    CheckResult r = check_brackets(source);
    if (r.ok) {
        printf("  [OK] %s\n", desc);
    } else {
        printf("  [오류] %s\n         줄 %d, 열 %d: %s\n",
               desc, r.error_line, r.error_col, r.error_msg);
    }
}

int main(void) {
    printf("=== 괄호 검사기 ===\n");

    test_case("단순 올바름",          "([{}])");
    test_case("복잡 올바름",          "({[()()]}[])");
    test_case("닫는 괄호 순서 오류",  "([)]");
    test_case("여는 괄호 미닫힘",     "((()");
    test_case("대응 없는 닫기",       ")abc(");
    test_case("빈 문자열",            "");

    /* 멀티라인 테스트 */
    const char *multiline =
        "int main() {\n"
        "    if (x > 0) {\n"
        "        printf(\"%d\", x];\n"  /* ] 가 잘못됨 */
        "    }\n"
        "}\n";
    test_case("멀티라인 코드 오류", multiline);

    const char *valid_code =
        "int main() {\n"
        "    for (int i = 0; i < 10; i++) {\n"
        "        func(arr[i]);\n"
        "    }\n"
        "    return 0;\n"
        "}\n";
    test_case("멀티라인 코드 정상", valid_code);

    return 0;
}
