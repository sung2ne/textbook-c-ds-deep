// filename: container_of_demo.c
// gcc -std=c17 -O2 -o container_of_demo container_of_demo.c

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Linux 커널 스타일 list_head */
struct list_head {
    struct list_head *prev;
    struct list_head *next;
};

/* container_of 매크로 정의 */
#define container_of(ptr, type, member) \
    ((type*)((char*)(ptr) - offsetof(type, member)))

/* list_head를 멤버로 갖는 사용자 데이터 구조체 */
typedef struct {
    int id;
    char name[32];
    struct list_head list;  /* 연결 포인터 */
} Student;

/* list_head 초기화 (자기 자신을 가리킴 = 빈 리스트) */
static inline void list_init(struct list_head *head) {
    head->prev = head;
    head->next = head;
}

/* head 뒤에 새 노드 삽입 (prepend) */
static inline void list_add(struct list_head *new_node,
                            struct list_head *head) {
    new_node->next = head->next;
    new_node->prev = head;
    head->next->prev = new_node;
    head->next = new_node;
}

/* 노드 삭제 */
static inline void list_del(struct list_head *entry) {
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
    entry->prev = NULL;
    entry->next = NULL;
}

/* list_for_each: 리스트 순회 매크로 */
#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/* list_for_each_entry: container_of와 결합한 순회 매크로 */
#define list_for_each_entry(pos, head, member) \
    for (pos = container_of((head)->next, typeof(*pos), member); \
         &(pos->member) != (head); \
         pos = container_of(pos->member.next, typeof(*pos), member))

int main(void) {
    /* sentinel 노드 (더미 헤드) */
    struct list_head students_list;
    list_init(&students_list);

    /* 학생 데이터 생성 */
    Student *s1 = malloc(sizeof(Student));
    Student *s2 = malloc(sizeof(Student));
    Student *s3 = malloc(sizeof(Student));

    s1->id = 1001; strcpy(s1->name, "김민준");
    s2->id = 1002; strcpy(s2->name, "이서연");
    s3->id = 1003; strcpy(s3->name, "박지호");

    /* 리스트에 삽입 */
    list_add(&s1->list, &students_list);
    list_add(&s2->list, &students_list);
    list_add(&s3->list, &students_list);

    /* container_of로 순회 */
    printf("=== 학생 목록 ===\n");
    struct list_head *pos;
    list_for_each(pos, &students_list) {
        /* pos는 list_head* — container_of로 Student* 복원 */
        Student *s = container_of(pos, Student, list);
        printf("  [%d] %s\n", s->id, s->name);
    }

    /* offsetof 확인 */
    printf("\n=== 메모리 레이아웃 ===\n");
    printf("offsetof(Student, id)   = %zu\n", offsetof(Student, id));
    printf("offsetof(Student, name) = %zu\n", offsetof(Student, name));
    printf("offsetof(Student, list) = %zu\n", offsetof(Student, list));
    printf("sizeof(Student)         = %zu\n", sizeof(Student));

    /* s1 삭제 후 재순회 */
    list_del(&s1->list);
    printf("\n=== s1 삭제 후 ===\n");
    list_for_each(pos, &students_list) {
        Student *s = container_of(pos, Student, list);
        printf("  [%d] %s\n", s->id, s->name);
    }

    free(s1); free(s2); free(s3);
    return 0;
}
