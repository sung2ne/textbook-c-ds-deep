// filename: priority_queue.c
// gcc -std=c17 -O2 -o priority_queue priority_queue.c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* ============================================================
 * 덱 (Deque) — sentinel 기반 양방향 연결 리스트
 * ============================================================ */
typedef struct DNode {
    void         *data;
    struct DNode *prev;
    struct DNode *next;
} DNode;

typedef struct {
    DNode  sentinel;  /* 더미 헤드 */
    size_t size;
} Deque;

static DNode *dnode_new(void *data) {
    DNode *n = malloc(sizeof(DNode));
    if (!n) { perror("malloc"); exit(1); }
    n->data = data;
    n->prev = n->next = NULL;
    return n;
}

static void deque_insert_between(Deque *dq, void *data,
                                  DNode *prev, DNode *next) {
    DNode *n = dnode_new(data);
    n->prev = prev; n->next = next;
    prev->next = n; next->prev = n;
    dq->size++;
}

void deque_init(Deque *dq) {
    dq->sentinel.prev = &dq->sentinel;
    dq->sentinel.next = &dq->sentinel;
    dq->size = 0;
}

bool deque_empty(const Deque *dq) { return dq->size == 0; }

void deque_push_front(Deque *dq, void *data) {
    deque_insert_between(dq, data, &dq->sentinel, dq->sentinel.next);
}

void deque_push_back(Deque *dq, void *data) {
    deque_insert_between(dq, data, dq->sentinel.prev, &dq->sentinel);
}

static void *deque_pop_node(Deque *dq, DNode *target) {
    if (!target || target == &dq->sentinel) return NULL;
    void *data = target->data;
    target->prev->next = target->next;
    target->next->prev = target->prev;
    free(target);
    dq->size--;
    return data;
}

void *deque_pop_front(Deque *dq) {
    return deque_pop_node(dq, dq->sentinel.next);
}

void *deque_pop_back(Deque *dq) {
    return deque_pop_node(dq, dq->sentinel.prev);
}

void *deque_peek_front(const Deque *dq) {
    if (deque_empty(dq)) return NULL;
    return dq->sentinel.next->data;
}

void *deque_peek_back(const Deque *dq) {
    if (deque_empty(dq)) return NULL;
    return dq->sentinel.prev->data;
}

void deque_destroy(Deque *dq) {
    while (!deque_empty(dq)) deque_pop_front(dq);
}

/* ============================================================
 * 우선순위 큐 — 최소 이진 힙
 * ============================================================ */
typedef struct {
    int   priority;  /* 낮을수록 높은 우선순위 (OS 스케줄러 관행) */
    char  name[64];
} PQItem;

typedef struct {
    PQItem *heap;
    size_t  size;
    size_t  capacity;
} PriorityQueue;

bool pq_init(PriorityQueue *pq, size_t capacity) {
    pq->heap = malloc(sizeof(PQItem) * capacity);
    if (!pq->heap) return false;
    pq->size = 0;
    pq->capacity = capacity;
    return true;
}

bool pq_empty(const PriorityQueue *pq) { return pq->size == 0; }

static void pq_swap(PriorityQueue *pq, size_t i, size_t j) {
    PQItem tmp = pq->heap[i];
    pq->heap[i] = pq->heap[j];
    pq->heap[j] = tmp;
}

/* siftUp: 인덱스 i의 원소를 적절한 위치까지 올림 */
static void sift_up(PriorityQueue *pq, size_t i) {
    while (i > 0) {
        size_t parent = (i - 1) / 2;
        if (pq->heap[parent].priority > pq->heap[i].priority) {
            pq_swap(pq, parent, i);
            i = parent;
        } else break;
    }
}

/* siftDown: 인덱스 i의 원소를 적절한 위치까지 내림 */
static void sift_down(PriorityQueue *pq, size_t i) {
    while (true) {
        size_t smallest = i;
        size_t left  = 2 * i + 1;
        size_t right = 2 * i + 2;

        if (left  < pq->size &&
            pq->heap[left].priority  < pq->heap[smallest].priority)
            smallest = left;
        if (right < pq->size &&
            pq->heap[right].priority < pq->heap[smallest].priority)
            smallest = right;

        if (smallest == i) break;
        pq_swap(pq, i, smallest);
        i = smallest;
    }
}

bool pq_push(PriorityQueue *pq, int priority, const char *name) {
    if (pq->size >= pq->capacity) {
        size_t new_cap = pq->capacity * 2;
        PQItem *new_heap = realloc(pq->heap, sizeof(PQItem) * new_cap);
        if (!new_heap) return false;
        pq->heap = new_heap;
        pq->capacity = new_cap;
    }
    pq->heap[pq->size].priority = priority;
    strncpy(pq->heap[pq->size].name, name, 63);
    pq->heap[pq->size].name[63] = '\0';
    sift_up(pq, pq->size);
    pq->size++;
    return true;
}

/* 최솟값 반환 후 제거 */
PQItem pq_pop(PriorityQueue *pq) {
    PQItem top = pq->heap[0];
    pq->heap[0] = pq->heap[--pq->size];
    if (pq->size > 0) sift_down(pq, 0);
    return top;
}

PQItem *pq_peek(PriorityQueue *pq) {
    return pq->size > 0 ? &pq->heap[0] : NULL;
}

void pq_destroy(PriorityQueue *pq) {
    free(pq->heap);
    pq->heap = NULL;
}

/* 힙 배열 시각화 */
void pq_print(const PriorityQueue *pq) {
    printf("  힙 배열: [");
    for (size_t i = 0; i < pq->size; i++) {
        printf("%d(%s)", pq->heap[i].priority, pq->heap[i].name);
        if (i < pq->size - 1) printf(", ");
    }
    printf("]\n");
}

/* ============================================================
 * 메인
 * ============================================================ */
int main(void) {
    /* 덱 테스트 */
    printf("=== 덱(Deque) 테스트 ===\n");
    Deque dq;
    deque_init(&dq);

    int vals[] = {1, 2, 3, 4, 5};
    deque_push_back(&dq,  &vals[0]);  /* [1] */
    deque_push_back(&dq,  &vals[1]);  /* [1, 2] */
    deque_push_front(&dq, &vals[2]);  /* [3, 1, 2] */
    deque_push_back(&dq,  &vals[3]);  /* [3, 1, 2, 4] */
    deque_push_front(&dq, &vals[4]);  /* [5, 3, 1, 2, 4] */

    printf("  front: %d\n", *(int*)deque_peek_front(&dq));
    printf("  back:  %d\n", *(int*)deque_peek_back(&dq));
    printf("  size:  %zu\n", dq.size);

    printf("  pop_front: %d\n", *(int*)deque_pop_front(&dq));
    printf("  pop_back:  %d\n", *(int*)deque_pop_back(&dq));
    printf("  size after 2 pops: %zu\n", dq.size);
    deque_destroy(&dq);

    /* 우선순위 큐 테스트 */
    printf("\n=== 우선순위 큐 테스트 (숫자 낮을수록 높은 우선순위) ===\n");
    PriorityQueue pq;
    pq_init(&pq, 8);

    /* OS 프로세스 스케줄링 시뮬레이션 */
    pq_push(&pq, 5, "배치 작업");
    pq_push(&pq, 1, "인터럽트 핸들러");
    pq_push(&pq, 3, "사용자 입력");
    pq_push(&pq, 2, "네트워크 I/O");
    pq_push(&pq, 4, "파일 쓰기");

    printf("  삽입 후 힙 상태:\n");
    pq_print(&pq);

    printf("\n  실행 순서 (우선순위 높은 순):\n");
    while (!pq_empty(&pq)) {
        PQItem item = pq_pop(&pq);
        printf("    [우선순위 %d] %s\n", item.priority, item.name);
    }

    /* siftUp 단계 추적 */
    printf("\n=== siftUp 추적 ===\n");
    pq_push(&pq, 10, "A");
    pq_push(&pq, 8,  "B");
    pq_push(&pq, 6,  "C");
    pq_push(&pq, 4,  "D");
    printf("  [10,8,6,4] 삽입 후: ");
    pq_print(&pq);

    printf("  우선순위 1인 원소 push:\n");
    pq_push(&pq, 1,  "최우선");
    printf("  siftUp 후: ");
    pq_print(&pq);  /* root가 1로 바뀌어야 함 */

    pq_destroy(&pq);
    return 0;
}
