// filename: task_scheduler.c
// gcc -std=c17 -O2 -o task_scheduler task_scheduler.c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

/* 작업 상태 */
typedef enum {
    TASK_PENDING,
    TASK_RUNNING,
    TASK_DONE,
    TASK_FAILED,
} TaskStatus;

/* 작업 구조체 */
typedef struct {
    int          priority;        /* 낮을수록 높은 우선순위 (0 = 최우선) */
    uint64_t     seq;             /* 등록 순서 (동일 우선순위 tiebreaker) */
    char         name[64];        /* 작업 이름 */
    TaskStatus   status;          /* 현재 상태 */
    void        *arg;             /* 작업 인자 */
    int        (*execute)(void*); /* 작업 함수 — 0=성공, 음수=실패 */
    void        (*on_done)(int, const char*); /* 완료 콜백 (결과코드, 이름) */
} Task;

/* 이진 힙 기반 우선순위 큐 (Task 포인터) */
typedef struct {
    Task  **heap;
    size_t  size;
    size_t  capacity;
} Scheduler;

static bool sched_init(Scheduler *s, size_t capacity) {
    s->heap = malloc(sizeof(Task*) * capacity);
    if (!s->heap) return false;
    s->size = 0;
    s->capacity = capacity;
    return true;
}

/* 비교: 우선순위 낮은 숫자 우선, 같으면 seq 낮은 것 우선 */
static bool sched_less(const Task *a, const Task *b) {
    if (a->priority != b->priority) return a->priority < b->priority;
    return a->seq < b->seq;
}

static void sched_swap(Scheduler *s, size_t i, size_t j) {
    Task *tmp = s->heap[i];
    s->heap[i] = s->heap[j];
    s->heap[j] = tmp;
}

static void sift_up(Scheduler *s, size_t i) {
    while (i > 0) {
        size_t p = (i - 1) / 2;
        if (sched_less(s->heap[i], s->heap[p])) {
            sched_swap(s, i, p);
            i = p;
        } else break;
    }
}

static void sift_down(Scheduler *s, size_t i) {
    while (true) {
        size_t best = i;
        size_t l = 2*i+1, r = 2*i+2;
        if (l < s->size && sched_less(s->heap[l], s->heap[best])) best = l;
        if (r < s->size && sched_less(s->heap[r], s->heap[best])) best = r;
        if (best == i) break;
        sched_swap(s, i, best);
        i = best;
    }
}

static uint64_t g_seq = 0;

bool sched_submit(Scheduler *s, int priority, const char *name,
                  int (*execute)(void*), void *arg,
                  void (*on_done)(int, const char*)) {
    if (s->size >= s->capacity) {
        size_t new_cap = s->capacity * 2;
        Task **new_heap = realloc(s->heap, sizeof(Task*) * new_cap);
        if (!new_heap) return false;
        s->heap = new_heap;
        s->capacity = new_cap;
    }
    Task *t = malloc(sizeof(Task));
    if (!t) return false;

    t->priority = priority;
    t->seq      = g_seq++;
    strncpy(t->name, name, 63); t->name[63] = '\0';
    t->status   = TASK_PENDING;
    t->arg      = arg;
    t->execute  = execute;
    t->on_done  = on_done;

    s->heap[s->size] = t;
    sift_up(s, s->size);
    s->size++;
    return true;
}

/* 가장 높은 우선순위 작업 1개 실행 */
bool sched_run_one(Scheduler *s) {
    if (s->size == 0) return false;

    Task *t = s->heap[0];
    s->heap[0] = s->heap[--s->size];
    if (s->size > 0) sift_down(s, 0);

    t->status = TASK_RUNNING;
    printf("  실행: [우선순위 %d] %s\n", t->priority, t->name);

    int result = t->execute(t->arg);
    t->status = (result == 0) ? TASK_DONE : TASK_FAILED;

    if (t->on_done) t->on_done(result, t->name);
    free(t);
    return true;
}

/* 모든 작업 실행 */
void sched_run_all(Scheduler *s) {
    while (s->size > 0) sched_run_one(s);
}

void sched_destroy(Scheduler *s) {
    for (size_t i = 0; i < s->size; i++) free(s->heap[i]);
    free(s->heap);
}

/* ---- 작업 함수들 ---- */
static int task_backup(void *arg) {
    const char *path = arg ? arg : "/data";
    printf("    → 백업 실행: %s\n", path);
    return 0;
}

static int task_security_patch(void *arg) {
    (void)arg;
    printf("    → 보안 패치 적용 중...\n");
    return 0;
}

static int task_log_compress(void *arg) {
    (void)arg;
    printf("    → 로그 파일 압축 완료\n");
    return 0;
}

static int task_cleanup(void *arg) {
    (void)arg;
    printf("    → 임시 파일 정리\n");
    return 0;
}

static int task_urgent(void *arg) {
    (void)arg;
    printf("    → 긴급 장애 대응!\n");
    return 0;
}

static void on_task_done(int result, const char *name) {
    printf("    완료: %s (%s)\n", name, result == 0 ? "성공" : "실패");
}

int main(void) {
    printf("=== 우선순위 기반 작업 스케줄러 ===\n\n");

    Scheduler s;
    sched_init(&s, 16);

    /* 작업 등록 (순서가 실행 순서가 아님) */
    sched_submit(&s, 5, "백업 실행",     task_backup,         "/backup", on_task_done);
    sched_submit(&s, 1, "보안 패치 적용", task_security_patch, NULL,      on_task_done);
    sched_submit(&s, 3, "로그 압축",     task_log_compress,   NULL,      on_task_done);
    sched_submit(&s, 5, "임시 파일 정리", task_cleanup,        NULL,      on_task_done);
    sched_submit(&s, 1, "긴급 장애 대응", task_urgent,         NULL,      on_task_done);

    printf("등록된 작업 수: %zu\n\n", s.size);

    printf("--- 스케줄러 실행 (높은 우선순위 = 낮은 숫자 먼저) ---\n");
    sched_run_all(&s);

    sched_destroy(&s);

    printf("\n--- 동일 우선순위 FIFO 확인 ---\n");
    sched_init(&s, 8);
    sched_submit(&s, 2, "작업 A (먼저 등록)", task_cleanup, NULL, NULL);
    sched_submit(&s, 2, "작업 B (나중 등록)", task_cleanup, NULL, NULL);
    sched_submit(&s, 2, "작업 C (마지막 등록)", task_cleanup, NULL, NULL);
    sched_run_all(&s);
    sched_destroy(&s);

    return 0;
}
