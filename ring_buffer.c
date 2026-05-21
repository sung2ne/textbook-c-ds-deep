// filename: ring_buffer.c
// gcc -std=c17 -O2 -o ring_buffer ring_buffer.c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/* 2의 제곱 여부 확인 */
static bool is_power_of_two(size_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

/* 다음 2의 제곱으로 올림 */
static size_t next_power_of_two(size_t n) {
    size_t p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- ring buffer (원형 큐) ---- */
typedef struct {
    void   **buf;      /* 포인터 배열 */
    size_t   capacity; /* 반드시 2의 제곱 */
    size_t   mask;     /* capacity - 1 */
    size_t   head;     /* 다음 dequeue 위치 */
    size_t   tail;     /* 다음 enqueue 위치 */
    size_t   count;    /* 현재 원소 수 */
} RingBuffer;

bool rb_init(RingBuffer *rb, size_t capacity) {
    if (!is_power_of_two(capacity))
        capacity = next_power_of_two(capacity);

    rb->buf = malloc(sizeof(void*) * capacity);
    if (!rb->buf) return false;

    rb->capacity = capacity;
    rb->mask     = capacity - 1;
    rb->head     = 0;
    rb->tail     = 0;
    rb->count    = 0;
    return true;
}

void rb_destroy(RingBuffer *rb) {
    free(rb->buf);
    rb->buf = NULL;
}

bool rb_empty(const RingBuffer *rb) { return rb->count == 0; }
bool rb_full(const RingBuffer *rb)  { return rb->count == rb->capacity; }
size_t rb_size(const RingBuffer *rb) { return rb->count; }

/* enqueue — rear(tail)에 삽입 */
bool rb_enqueue(RingBuffer *rb, void *data) {
    if (rb_full(rb)) {
        /* 동적 확장: capacity를 두 배로 */
        size_t new_cap = rb->capacity * 2;
        void **new_buf = malloc(sizeof(void*) * new_cap);
        if (!new_buf) return false;

        /* head부터 순서대로 복사 */
        for (size_t i = 0; i < rb->count; i++)
            new_buf[i] = rb->buf[(rb->head + i) & rb->mask];

        free(rb->buf);
        rb->buf      = new_buf;
        rb->head     = 0;
        rb->tail     = rb->count;
        rb->capacity = new_cap;
        rb->mask     = new_cap - 1;
    }

    rb->buf[rb->tail & rb->mask] = data;
    rb->tail++;   /* 증가만 하고 mask로 wrap */
    rb->count++;
    return true;
}

/* dequeue — front(head)에서 꺼냄 */
void *rb_dequeue(RingBuffer *rb) {
    if (rb_empty(rb)) return NULL;
    void *data = rb->buf[rb->head & rb->mask];
    rb->head++;
    rb->count--;
    return data;
}

/* front 값 확인 (제거하지 않음) */
void *rb_peek(const RingBuffer *rb) {
    if (rb_empty(rb)) return NULL;
    return rb->buf[rb->head & rb->mask];
}

/* 내부 상태 출력 (디버깅용) */
void rb_debug(const RingBuffer *rb, void (*printer)(const void *)) {
    printf("  capacity=%zu count=%zu head=%zu tail=%zu\n",
           rb->capacity, rb->count,
           rb->head & rb->mask, rb->tail & rb->mask);
    printf("  [");
    for (size_t i = 0; i < rb->count; i++) {
        printer(rb->buf[(rb->head + i) & rb->mask]);
        if (i < rb->count - 1) printf(", ");
    }
    printf("]\n");
}

/* ---- 메시지 큐 시뮬레이션 ---- */
typedef struct {
    int   id;
    char  msg[64];
} Message;

static Message *msg_new(int id, const char *text) {
    Message *m = malloc(sizeof(Message));
    m->id = id;
    strncpy(m->msg, text, 63);
    m->msg[63] = '\0';
    return m;
}

static void msg_print(const void *data) {
    const Message *m = data;
    printf("M%d(\"%s\")", m->id, m->msg);
}

int main(void) {
    RingBuffer rb;
    rb_init(&rb, 4);  /* 초기 capacity=4 */

    printf("=== 메시지 큐 시뮬레이션 ===\n");

    /* 4개 enqueue */
    for (int i = 1; i <= 4; i++) {
        char text[32];
        snprintf(text, sizeof(text), "요청 %d", i);
        rb_enqueue(&rb, msg_new(i, text));
    }
    printf("4개 enqueue 후:\n");
    rb_debug(&rb, msg_print);

    /* 2개 dequeue */
    for (int i = 0; i < 2; i++) {
        Message *m = rb_dequeue(&rb);
        printf("dequeue: M%d \"%s\"\n", m->id, m->msg);
        free(m);
    }
    printf("2개 dequeue 후:\n");
    rb_debug(&rb, msg_print);

    /* 3개 더 enqueue (wrap-around 발생) */
    for (int i = 5; i <= 7; i++) {
        char text[32];
        snprintf(text, sizeof(text), "요청 %d", i);
        rb_enqueue(&rb, msg_new(i, text));
    }
    printf("3개 추가 enqueue (wrap-around 발생):\n");
    rb_debug(&rb, msg_print);

    /* capacity 초과 → 자동 확장 */
    rb_enqueue(&rb, msg_new(8, "요청 8"));
    printf("capacity 초과 → 자동 확장:\n");
    rb_debug(&rb, msg_print);

    /* 2의 제곱 최적화 확인 */
    printf("\n=== 2의 제곱 최적화 확인 ===\n");
    printf("capacity = %zu = 2^%d\n", rb.capacity,
           __builtin_ctzll(rb.capacity));
    printf("mask     = %zu = 0x%zx\n", rb.mask, rb.mask);
    printf("index wrap: (tail+1) & mask = %zu\n",
           (rb.tail + 1) & rb.mask);

    /* 전체 비우기 */
    while (!rb_empty(&rb)) {
        Message *m = rb_dequeue(&rb);
        free(m);
    }
    rb_destroy(&rb);

    /* Linux kfifo 스타일: head/tail을 절대 인덱스로 관리 */
    printf("\n=== head/tail 절대 인덱스 방식 ===\n");
    printf("head와 tail은 오버플로우를 허용 — mask로 wrap\n");
    printf("count = tail - head (unsigned 연산으로 항상 올바름)\n");
    uint32_t h = 0xFFFFFFFE, t = 0;  /* 오버플로우 직전 */
    t++;  /* enqueue */
    printf("head=0x%08X tail=0x%08X count=%u\n", h, t, t - h);

    return 0;
}
