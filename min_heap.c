// filename: min_heap.c
// gcc -std=c17 -O2 -o min_heap min_heap.c

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define HEAP_INIT_CAP 16

/* ── Min Heap 구조체 ── */
typedef struct {
    int *data;
    int  size;
    int  capacity;
} MinHeap;

/* ── 초기화 ── */
void heap_init(MinHeap *h) {
    h->data     = malloc(HEAP_INIT_CAP * sizeof(int));
    h->size     = 0;
    h->capacity = HEAP_INIT_CAP;
}

/* ── 동적 확장 ── */
static void heap_grow(MinHeap *h) {
    h->capacity *= 2;
    h->data = realloc(h->data, h->capacity * sizeof(int));
}

/* ── 스왑 ── */
static void swap(int *a, int *b) {
    int tmp = *a; *a = *b; *b = tmp;
}

/* ── siftUp: 삽입 후 위로 올라가며 힙 속성 복원 ── */
static void sift_up(MinHeap *h, int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (h->data[parent] <= h->data[i]) break;
        swap(&h->data[parent], &h->data[i]);
        i = parent;
    }
}

/* ── siftDown: 삭제 후 아래로 내려가며 힙 속성 복원 ── */
static void sift_down(MinHeap *h, int i) {
    while (1) {
        int smallest = i;
        int left     = 2 * i + 1;
        int right    = 2 * i + 2;

        if (left  < h->size && h->data[left]  < h->data[smallest])
            smallest = left;
        if (right < h->size && h->data[right] < h->data[smallest])
            smallest = right;

        if (smallest == i) break;

        swap(&h->data[smallest], &h->data[i]);
        i = smallest;
    }
}

/* ── 삽입: O(log n) ── */
void heap_push(MinHeap *h, int val) {
    if (h->size == h->capacity) heap_grow(h);
    h->data[h->size] = val;
    sift_up(h, h->size);
    h->size++;
}

/* ── 최솟값 조회: O(1) ── */
int heap_top(const MinHeap *h) {
    assert(h->size > 0);
    return h->data[0];
}

/* ── 최솟값 삭제: O(log n) ── */
int heap_pop(MinHeap *h) {
    assert(h->size > 0);
    int min = h->data[0];
    h->size--;
    h->data[0] = h->data[h->size];  /* 마지막 원소를 루트로 */
    sift_down(h, 0);
    return min;
}

/* ── 배열에서 Heap 구성: O(n) ── */
void heap_build(MinHeap *h, int *arr, int n) {
    if (n > h->capacity) {
        free(h->data);
        h->data     = malloc(n * sizeof(int));
        h->capacity = n;
    }
    for (int i = 0; i < n; i++) h->data[i] = arr[i];
    h->size = n;
    /* 마지막 내부 노드부터 루트까지 siftDown */
    for (int i = (n - 2) / 2; i >= 0; i--)
        sift_down(h, i);
}

void heap_free(MinHeap *h) { free(h->data); }

int main(void) {
    MinHeap h;
    heap_init(&h);

    int values[] = {5, 3, 8, 1, 9, 2, 7, 4, 6};
    for (int i = 0; i < 9; i++)
        heap_push(&h, values[i]);

    printf("정렬 출력: ");
    while (h.size > 0)
        printf("%d ", heap_pop(&h));
    printf("\n");

    /* heap_build: O(n) 일괄 구성 */
    int arr[] = {10, 20, 5, 15, 3, 8};
    heap_build(&h, arr, 6);
    printf("heap_build 최솟값: %d\n", heap_top(&h));

    heap_free(&h);
    return 0;
}
