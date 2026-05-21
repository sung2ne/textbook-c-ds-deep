// filename: event_scheduler.c
// gcc -std=c17 -O2 -o event_scheduler event_scheduler.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_EVENTS 256

/* ── 이벤트 구조체 ── */
typedef struct {
    time_t  trigger_at;          /* 실행할 Unix timestamp */
    char    name[64];
    void  (*handler)(void *arg); /* 콜백 함수 */
    void   *arg;
} Event;

/* ── Min-Heap (trigger_at 기준) ── */
typedef struct {
    Event data[MAX_EVENTS];
    int   size;
} EventHeap;

static void swap_event(Event *a, Event *b) {
    Event tmp = *a; *a = *b; *b = tmp;
}

static void sift_up(EventHeap *h, int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (h->data[parent].trigger_at <= h->data[i].trigger_at) break;
        swap_event(&h->data[parent], &h->data[i]);
        i = parent;
    }
}

static void sift_down(EventHeap *h, int i) {
    while (1) {
        int earliest = i;
        int l = 2 * i + 1, r = 2 * i + 2;
        if (l < h->size && h->data[l].trigger_at < h->data[earliest].trigger_at)
            earliest = l;
        if (r < h->size && h->data[r].trigger_at < h->data[earliest].trigger_at)
            earliest = r;
        if (earliest == i) break;
        swap_event(&h->data[earliest], &h->data[i]);
        i = earliest;
    }
}

/* ── 이벤트 등록 ── */
void scheduler_add(EventHeap *h, time_t trigger_at,
                   const char *name, void (*handler)(void *), void *arg) {
    if (h->size >= MAX_EVENTS) return;
    Event *e = &h->data[h->size];
    e->trigger_at = trigger_at;
    strncpy(e->name, name, sizeof(e->name) - 1);
    e->handler    = handler;
    e->arg        = arg;
    sift_up(h, h->size);
    h->size++;
}

/* ── tick: 현재 시간 이전 이벤트 모두 실행 ── */
void scheduler_tick(EventHeap *h, time_t now) {
    while (h->size > 0 && h->data[0].trigger_at <= now) {
        Event e = h->data[0];          /* 최이른 이벤트 복사 */
        h->size--;
        h->data[0] = h->data[h->size]; /* 마지막 원소를 루트로 */
        sift_down(h, 0);

        printf("[%ld] 이벤트 실행: %s\n", (long)now, e.name);
        if (e.handler) e.handler(e.arg);
    }
}

/* ── 핸들러 예시 ── */
static void on_session_expire(void *arg) {
    printf("  → 세션 만료 처리: user_id=%d\n", *(int *)arg);
}

static void on_cache_flush(void *arg) {
    (void)arg;
    printf("  → 캐시 플러시 완료\n");
}

static void on_reconnect(void *arg) {
    printf("  → 재연결 시도: endpoint=%s\n", (char *)arg);
}

int main(void) {
    EventHeap scheduler = { .size = 0 };
    time_t    now       = 1000;  /* 기준 시간 (임의) */

    static int  user_id   = 42;
    static char endpoint[] = "redis://127.0.0.1:6379";

    /* 이벤트 등록 */
    scheduler_add(&scheduler, now + 5,  "세션 만료",  on_session_expire, &user_id);
    scheduler_add(&scheduler, now + 2,  "재연결",     on_reconnect,      endpoint);
    scheduler_add(&scheduler, now + 10, "캐시 플러시", on_cache_flush,   NULL);
    scheduler_add(&scheduler, now + 1,  "헬스체크",   NULL,              NULL);

    printf("=== 이벤트 스케줄러 시뮬레이션 ===\n");

    /* 시간 흐름 시뮬레이션 */
    for (time_t t = now; t <= now + 12; t++) {
        scheduler_tick(&scheduler, t);
    }

    return 0;
}
