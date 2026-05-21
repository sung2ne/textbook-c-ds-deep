// filename: stack.c
// gcc -std=c17 -O2 -o stack stack.c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

/* ---- 배열 기반 스택 ---- */
typedef struct {
    void **data;   /* void* 배열 */
    int   top;
    int   capacity;
} ArrayStack;

bool astack_init(ArrayStack *s, int capacity) {
    s->data = malloc(sizeof(void*) * (size_t)capacity);
    if (!s->data) return false;
    s->top = -1;
    s->capacity = capacity;
    return true;
}

bool astack_push(ArrayStack *s, void *item) {
    if (s->top >= s->capacity - 1) {
        /* 동적 확장 */
        int new_cap = s->capacity * 2;
        void **new_data = realloc(s->data, sizeof(void*) * (size_t)new_cap);
        if (!new_data) return false;
        s->data = new_data;
        s->capacity = new_cap;
    }
    s->data[++s->top] = item;
    return true;
}

void *astack_pop(ArrayStack *s) {
    if (s->top < 0) return NULL;
    return s->data[s->top--];
}

void *astack_peek(const ArrayStack *s) {
    if (s->top < 0) return NULL;
    return s->data[s->top];
}

bool astack_empty(const ArrayStack *s) { return s->top < 0; }
int  astack_size(const ArrayStack *s)  { return s->top + 1; }
void astack_destroy(ArrayStack *s)     { free(s->data); s->data = NULL; }

/* ---- Shunting-yard: 중위 → 후위 변환 ---- */
static int precedence(char op) {
    switch (op) {
        case '+': case '-': return 1;
        case '*': case '/': return 2;
        case '^':           return 3;
        default:            return 0;
    }
}

static bool is_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

/* result 버퍼에 후위 표기법 문자열 작성, 성공 시 true 반환 */
bool infix_to_postfix(const char *infix, char *result, size_t result_size) {
    ArrayStack op_stack;
    if (!astack_init(&op_stack, 32)) return false;

    size_t pos = 0;
    for (const char *p = infix; *p; p++) {
        if (*p == ' ') continue;

        if (isdigit((unsigned char)*p)) {
            /* 다자리 숫자 처리 */
            while (isdigit((unsigned char)*p)) {
                if (pos < result_size - 1) result[pos++] = *p++;
                else { astack_destroy(&op_stack); return false; }
            }
            p--;  /* for 루프의 p++와 상쇄 */
            if (pos < result_size - 1) result[pos++] = ' ';
        } else if (*p == '(') {
            astack_push(&op_stack, (void*)(intptr_t)'(');
        } else if (*p == ')') {
            while (!astack_empty(&op_stack)) {
                char top = (char)(intptr_t)astack_peek(&op_stack);
                if (top == '(') { astack_pop(&op_stack); break; }
                if (pos < result_size - 2) {
                    result[pos++] = (char)(intptr_t)astack_pop(&op_stack);
                    result[pos++] = ' ';
                }
            }
        } else if (is_operator(*p)) {
            while (!astack_empty(&op_stack)) {
                char top = (char)(intptr_t)astack_peek(&op_stack);
                if (top == '(') break;
                if (precedence(top) >= precedence(*p)) {
                    if (pos < result_size - 2) {
                        result[pos++] = (char)(intptr_t)astack_pop(&op_stack);
                        result[pos++] = ' ';
                    }
                } else break;
            }
            astack_push(&op_stack, (void*)(intptr_t)*p);
        }
    }

    /* 스택에 남은 연산자 출력 */
    while (!astack_empty(&op_stack)) {
        if (pos < result_size - 2) {
            result[pos++] = (char)(intptr_t)astack_pop(&op_stack);
            result[pos++] = ' ';
        }
    }
    if (pos > 0 && result[pos-1] == ' ') pos--;  /* 마지막 공백 제거 */
    result[pos] = '\0';

    astack_destroy(&op_stack);
    return true;
}

/* 후위 표기법 계산 */
double eval_postfix(const char *expr) {
    ArrayStack num_stack;
    astack_init(&num_stack, 32);

    const char *p = expr;
    while (*p) {
        if (*p == ' ') { p++; continue; }
        if (isdigit((unsigned char)*p)) {
            double val = 0;
            while (isdigit((unsigned char)*p)) val = val * 10 + (*p++ - '0');
            /* double을 void*에 넣기 위해 union 사용 */
            union { double d; void *v; } u;
            u.d = val;
            astack_push(&num_stack, u.v);
        } else if (is_operator(*p)) {
            union { double d; void *v; } b, a, res;
            b.v = astack_pop(&num_stack);
            a.v = astack_pop(&num_stack);
            switch (*p) {
                case '+': res.d = a.d + b.d; break;
                case '-': res.d = a.d - b.d; break;
                case '*': res.d = a.d * b.d; break;
                case '/': res.d = b.d != 0 ? a.d / b.d : 0; break;
                default:  res.d = 0;
            }
            astack_push(&num_stack, res.v);
            p++;
        } else p++;
    }

    union { double d; void *v; } result;
    result.v = astack_pop(&num_stack);
    astack_destroy(&num_stack);
    return result.d;
}

/* 괄호 유효성 검사 */
bool check_brackets(const char *s) {
    ArrayStack st;
    astack_init(&st, 64);
    bool ok = true;
    for (; *s && ok; s++) {
        if (*s == '(' || *s == '[' || *s == '{') {
            astack_push(&st, (void*)(intptr_t)*s);
        } else if (*s == ')' || *s == ']' || *s == '}') {
            if (astack_empty(&st)) { ok = false; break; }
            char top = (char)(intptr_t)astack_pop(&st);
            if ((*s == ')' && top != '(') ||
                (*s == ']' && top != '[') ||
                (*s == '}' && top != '{')) {
                ok = false;
            }
        }
    }
    if (!astack_empty(&st)) ok = false;
    astack_destroy(&st);
    return ok;
}

int main(void) {
    /* 수식 변환 테스트 */
    const char *expressions[] = {
        "3 + 4 * 2",
        "(3 + 4) * 2",
        "3 + 4 * 2 / (1 - 5)",
        "2 + 3 * 4 - 1",
    };

    printf("=== 중위 → 후위 변환 ===\n");
    char postfix[256];
    for (int i = 0; i < 4; i++) {
        if (infix_to_postfix(expressions[i], postfix, sizeof(postfix))) {
            double result = eval_postfix(postfix);
            printf("  %s\n    → %s = %.2f\n",
                   expressions[i], postfix, result);
        }
    }

    /* 괄호 검사 */
    printf("\n=== 괄호 유효성 검사 ===\n");
    const char *bracket_tests[] = {
        "([{}])",
        "([)]",
        "{[}",
        "((()))",
        "",
    };
    for (int i = 0; i < 5; i++) {
        printf("  \"%s\" → %s\n",
               bracket_tests[i],
               check_brackets(bracket_tests[i]) ? "유효" : "무효");
    }

    return 0;
}
