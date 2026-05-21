// filename: cas_aba_demo.c
// ABA 문제 시연 (단일 스레드, 개념 설명용)
// gcc -std=c17 -O2 -o cas_aba_demo cas_aba_demo.c
#include <stdio.h>
#include <stdatomic.h>
#include <stdlib.h>

typedef struct Node {
    int value;
    struct Node *next;
} Node;

/* ── ABA에 취약한 Lock-Free 스택 ────────────── */
typedef struct {
    _Atomic(Node *) head;
} UnsafeStack;

void unsafe_push(UnsafeStack *s, Node *n) {
    Node *old_head;
    do {
        old_head = atomic_load(&s->head);
        n->next = old_head;
    } while (!atomic_compare_exchange_weak(&s->head, &old_head, n));
}

Node *unsafe_pop(UnsafeStack *s) {
    Node *old_head, *new_head;
    do {
        old_head = atomic_load(&s->head);
        if (!old_head) return NULL;
        new_head = old_head->next;
    } while (!atomic_compare_exchange_weak(&s->head, &old_head, new_head));
    return old_head;
}

/* ── Tagged pointer로 ABA 해결 ───────────────── */
/* 포인터의 하위 비트나 상위 비트를 버전 카운터로 사용 */
/* 아래는 uintptr_t로 포인터와 태그를 합치는 방식 */
#include <stdint.h>

typedef struct {
    _Atomic uintptr_t tagged;   /* 하위 2비트 = 태그(버전), 나머지 = 포인터 */
} TaggedStack;

#define TAG_MASK   0x3ULL          /* 하위 2비트 */
#define PTR_MASK   (~TAG_MASK)     /* 나머지 비트 */
#define GET_PTR(v)  ((Node *)((v) & PTR_MASK))
#define GET_TAG(v)  ((v) & TAG_MASK)
#define MAKE_TV(p, t) (((uintptr_t)(p) & PTR_MASK) | ((t) & TAG_MASK))

void tagged_push(TaggedStack *s, Node *n) {
    uintptr_t old_tv, new_tv;
    do {
        old_tv = atomic_load(&s->tagged);
        n->next = GET_PTR(old_tv);
        new_tv = MAKE_TV(n, GET_TAG(old_tv) + 1);  /* 태그 증가 */
    } while (!atomic_compare_exchange_weak(&s->tagged, &old_tv, new_tv));
}

Node *tagged_pop(TaggedStack *s) {
    uintptr_t old_tv, new_tv;
    Node *old_head;
    do {
        old_tv = atomic_load(&s->tagged);
        old_head = GET_PTR(old_tv);
        if (!old_head) return NULL;
        new_tv = MAKE_TV(old_head->next, GET_TAG(old_tv) + 1);
    } while (!atomic_compare_exchange_weak(&s->tagged, &old_tv, new_tv));
    return old_head;
}

int main(void) {
    /* ABA 취약 스택 테스트 */
    UnsafeStack us = { .head = ATOMIC_VAR_INIT(NULL) };
    Node a = {1, NULL}, b = {2, NULL}, c = {3, NULL};

    unsafe_push(&us, &c);
    unsafe_push(&us, &b);
    unsafe_push(&us, &a);   /* 스택: A -> B -> C */
    printf("Unsafe Stack top: %d\n", atomic_load(&us.head)->value);

    /* Tagged pointer 스택 테스트 */
    TaggedStack ts = { .tagged = ATOMIC_VAR_INIT(0) };
    Node x = {10, NULL}, y = {20, NULL};

    tagged_push(&ts, &y);
    tagged_push(&ts, &x);   /* 스택: X -> Y */
    Node *popped = tagged_pop(&ts);
    printf("Tagged Stack pop: %d\n", popped ? popped->value : -1);
    printf("Tagged Stack top: %d\n", GET_PTR(atomic_load(&ts.tagged))->value);

    printf("\nABA 개념 설명:\n");
    printf("T1: head = A를 읽음\n");
    printf("T2: pop A, pop B, push A (같은 포인터 재사용!)\n");
    printf("T1: CAS(A, B) 성공 → B는 해제된 메모리!\n");
    printf("Tagged pointer: 태그가 달라서 CAS 실패 → ABA 방지\n");

    return 0;
}
