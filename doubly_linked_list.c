// filename: doubly_linked_list.c
// gcc -std=c17 -O2 -o doubly_linked_list doubly_linked_list.c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

/* ---- Linux 스타일 list_head ---- */
struct list_head {
    struct list_head *prev;
    struct list_head *next;
};

#define container_of(ptr, type, member) \
    ((type*)((char*)(ptr) - offsetof(type, member)))

#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

static inline void list_init(struct list_head *head) {
    head->prev = head;
    head->next = head;
}

static inline bool list_empty(const struct list_head *head) {
    return head->next == head;
}

static inline void __list_add(struct list_head *new_node,
                               struct list_head *prev,
                               struct list_head *next) {
    next->prev = new_node;
    new_node->next = next;
    new_node->prev = prev;
    prev->next = new_node;
}

static inline void list_add(struct list_head *new_node,
                             struct list_head *head) {
    __list_add(new_node, head, head->next);
}

static inline void list_add_tail(struct list_head *new_node,
                                  struct list_head *head) {
    __list_add(new_node, head->prev, head);
}

static inline void list_del(struct list_head *entry) {
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
    entry->prev = NULL;
    entry->next = NULL;
}

/* 노드를 한 위치 뒤로 이동 (swap with next) */
static inline void list_move_tail(struct list_head *list,
                                   struct list_head *head) {
    list_del(list);
    list_add_tail(list, head);
}

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; \
         pos != (head); \
         pos = n, n = pos->next)

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

/* ---- 사용자 데이터 구조체 ---- */
typedef struct {
    int priority;
    char task_name[64];
    struct list_head list;
} Task;

static Task *task_new(int priority, const char *name) {
    Task *t = malloc(sizeof(Task));
    if (!t) { perror("malloc"); exit(1); }
    t->priority = priority;
    strncpy(t->task_name, name, 63);
    t->task_name[63] = '\0';
    list_init(&t->list);
    return t;
}

static void task_free(Task *t) { free(t); }

/* 우선순위 순으로 삽입 (높은 숫자 = 높은 우선순위) */
static void task_list_insert_sorted(struct list_head *head, Task *new_task) {
    struct list_head *pos;
    list_for_each(pos, head) {
        Task *t = list_entry(pos, Task, list);
        if (new_task->priority > t->priority) {
            /* new_task를 pos 앞에 삽입 */
            __list_add(&new_task->list, pos->prev, pos);
            return;
        }
    }
    /* 가장 낮은 우선순위 → tail에 추가 */
    list_add_tail(&new_task->list, head);
}

static void task_list_print(struct list_head *head) {
    if (list_empty(head)) {
        printf("  (비어 있음)\n");
        return;
    }
    struct list_head *pos;
    list_for_each(pos, head) {
        Task *t = list_entry(pos, Task, list);
        printf("  [우선순위 %d] %s\n", t->priority, t->task_name);
    }
}

/* ---- 메인 ---- */
int main(void) {
    LIST_HEAD(task_queue);  /* sentinel 노드 스택에 생성 */

    /* 작업 추가 */
    Task *tasks[] = {
        task_new(3, "백업 실행"),
        task_new(9, "보안 패치 적용"),
        task_new(5, "로그 압축"),
        task_new(7, "DB 쿼리 최적화"),
        task_new(1, "임시 파일 정리"),
    };
    for (int i = 0; i < 5; i++) {
        task_list_insert_sorted(&task_queue, tasks[i]);
    }

    printf("=== 우선순위 순서 ===\n");
    task_list_print(&task_queue);

    /* 상위 2개 작업 처리 (head에서 꺼냄) */
    printf("\n=== 상위 2개 처리 ===\n");
    for (int i = 0; i < 2; i++) {
        if (!list_empty(&task_queue)) {
            Task *t = list_entry(task_queue.next, Task, list);
            printf("  처리: [%d] %s\n", t->priority, t->task_name);
            list_del(&t->list);
            task_free(t);
        }
    }

    printf("\n=== 남은 작업 ===\n");
    task_list_print(&task_queue);

    /* 나머지 해제 */
    struct list_head *pos, *n;
    list_for_each_safe(pos, n, &task_queue) {
        Task *t = list_entry(pos, Task, list);
        list_del(&t->list);
        task_free(t);
    }

    printf("\n빈 리스트: %s\n", list_empty(&task_queue) ? "예" : "아니오");

    /* sentinel 크기 확인 */
    printf("\nsizeof(list_head) = %zu 바이트\n", sizeof(struct list_head));
    printf("sizeof(Task)      = %zu 바이트\n", sizeof(Task));
    printf("offsetof(Task, list) = %zu\n", offsetof(Task, list));

    return 0;
}
