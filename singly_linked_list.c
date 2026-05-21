// filename: singly_linked_list.c
// gcc -std=c17 -O2 -o singly_linked_list singly_linked_list.c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
    void *data;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    size_t size;
} SinglyLinkedList;

/* 새 노드 생성 */
static Node *node_new(void *data) {
    Node *n = malloc(sizeof(Node));
    if (!n) { fprintf(stderr, "malloc failed\n"); exit(1); }
    n->data = data;
    n->next = NULL;
    return n;
}

/* 리스트 초기화 */
void sll_init(SinglyLinkedList *list) {
    list->head = NULL;
    list->size = 0;
}

/* head 앞에 삽입 — O(1) */
void sll_prepend(SinglyLinkedList *list, void *data) {
    Node *n = node_new(data);
    n->next = list->head;
    list->head = n;
    list->size++;
}

/* tail 뒤에 삽입 — O(n) */
void sll_append(SinglyLinkedList *list, void *data) {
    Node *n = node_new(data);
    if (!list->head) {
        list->head = n;
    } else {
        Node *cur = list->head;
        while (cur->next) cur = cur->next;
        cur->next = n;
    }
    list->size++;
}

/* index 위치에 삽입 — O(n) */
bool sll_insert_at(SinglyLinkedList *list, size_t index, void *data) {
    if (index > list->size) return false;
    if (index == 0) { sll_prepend(list, data); return true; }

    Node *cur = list->head;
    for (size_t i = 0; i < index - 1; i++) cur = cur->next;

    Node *n = node_new(data);
    n->next = cur->next;   /* 먼저 연결 */
    cur->next = n;         /* 그다음 연결 */
    list->size++;
    return true;
}

/* 첫 번째 일치 노드 삭제 (comparator 기반) */
bool sll_remove(SinglyLinkedList *list, void *data,
                int (*cmp)(const void *, const void *)) {
    Node **pp = &list->head;  /* 이중 포인터: 이전 노드의 next 필드를 가리킴 */
    while (*pp) {
        if (cmp((*pp)->data, data) == 0) {
            Node *target = *pp;
            *pp = target->next;   /* 이전 노드의 next를 건너뜀 */
            free(target);
            list->size--;
            return true;
        }
        pp = &(*pp)->next;
    }
    return false;
}

/* 리스트 역순 (제자리) */
void sll_reverse(SinglyLinkedList *list) {
    Node *prev = NULL;
    Node *cur  = list->head;
    while (cur) {
        Node *next_node = cur->next;
        cur->next = prev;
        prev = cur;
        cur = next_node;
    }
    list->head = prev;
}

/* Floyd의 사이클 감지 — O(n) 시간, O(1) 공간 */
bool sll_has_cycle(const SinglyLinkedList *list) {
    Node *slow = list->head;  /* 거북이: 한 칸씩 */
    Node *fast = list->head;  /* 토끼: 두 칸씩 */
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) return true;  /* 만남 = 사이클 */
    }
    return false;
}

/* 순회 출력 (printer 콜백) */
void sll_print(const SinglyLinkedList *list,
               void (*printer)(const void *)) {
    printf("[");
    Node *cur = list->head;
    while (cur) {
        printer(cur->data);
        if (cur->next) printf(" → ");
        cur = cur->next;
    }
    printf("] (size=%zu)\n", list->size);
}

/* 리스트 메모리 해제 (data는 해제하지 않음) */
void sll_destroy(SinglyLinkedList *list) {
    Node *cur = list->head;
    while (cur) {
        Node *next_node = cur->next;
        free(cur);
        cur = next_node;
    }
    list->head = NULL;
    list->size = 0;
}

/* --- 테스트 --- */
static int int_cmp(const void *a, const void *b) {
    return *(int*)a - *(int*)b;
}

static void int_print(const void *data) {
    printf("%d", *(int*)data);
}

int main(void) {
    SinglyLinkedList list;
    sll_init(&list);

    int values[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) sll_append(&list, &values[i]);

    printf("초기 리스트: ");
    sll_print(&list, int_print);

    sll_prepend(&list, &(int){5});
    printf("prepend(5): ");
    sll_print(&list, int_print);

    sll_insert_at(&list, 3, &(int){25});
    printf("insert_at(3, 25): ");
    sll_print(&list, int_print);

    sll_remove(&list, &(int){30}, int_cmp);
    printf("remove(30): ");
    sll_print(&list, int_print);

    sll_reverse(&list);
    printf("reverse: ");
    sll_print(&list, int_print);

    printf("사이클 있음: %s\n", sll_has_cycle(&list) ? "예" : "아니오");

    /* 인위적으로 사이클 생성 (테스트용) */
    Node *tail = list.head;
    while (tail->next) tail = tail->next;
    tail->next = list.head->next;  /* 마지막 → 두 번째 */
    printf("사이클 생성 후: %s\n", sll_has_cycle(&list) ? "예" : "아니오");

    /* 사이클 제거 후 정상 해제 */
    tail->next = NULL;
    sll_destroy(&list);
    return 0;
}
