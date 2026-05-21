// filename: vector.c
// gcc -std=c17 -O2 -c vector.c

#include "vector.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

/* 내부 헬퍼: idx번째 원소의 주소 */
static void* elem_ptr(const vector_t* v, size_t idx) {
    return (char*)v->data + idx * v->elem_size;
}

/* 내부 헬퍼: capacity 확장 */
static void grow(vector_t* v) {
    size_t new_cap = (v->capacity == 0) ? 4 : v->capacity * 2;
    void* tmp = realloc(v->data, new_cap * v->elem_size);
    if (tmp == NULL) {
        fprintf(stderr, "vector: 메모리 부족\n");
        exit(EXIT_FAILURE);
    }
    v->data = tmp;
    v->capacity = new_cap;
}

vector_t* vec_create(size_t elem_size, size_t initial_cap) {
    assert(elem_size > 0);
    vector_t* v = malloc(sizeof(vector_t));
    if (!v) return NULL;

    v->elem_size = elem_size;
    v->size      = 0;
    v->capacity  = initial_cap;

    if (initial_cap > 0) {
        v->data = malloc(elem_size * initial_cap);
        if (!v->data) { free(v); return NULL; }
    } else {
        v->data = NULL;
    }
    return v;
}

void vec_destroy(vector_t* v) {
    if (!v) return;
    free(v->data);
    v->data = NULL;
    free(v);
}

void vec_push(vector_t* v, const void* elem) {
    assert(v && elem);
    if (v->size == v->capacity) grow(v);
    memcpy(elem_ptr(v, v->size), elem, v->elem_size);
    v->size++;
}

void vec_pop(vector_t* v, void* out) {
    assert(v && v->size > 0);
    v->size--;
    if (out) memcpy(out, elem_ptr(v, v->size), v->elem_size);
}

void* vec_get(const vector_t* v, size_t idx) {
    assert(v && idx < v->size);
    return elem_ptr(v, idx);
}

void vec_set(vector_t* v, size_t idx, const void* elem) {
    assert(v && idx < v->size && elem);
    memcpy(elem_ptr(v, idx), elem, v->elem_size);
}

void vec_insert(vector_t* v, size_t idx, const void* elem) {
    assert(v && idx <= v->size && elem);
    if (v->size == v->capacity) grow(v);
    /* idx 이후 원소를 한 칸씩 오른쪽으로 */
    if (idx < v->size) {
        memmove(elem_ptr(v, idx + 1),
                elem_ptr(v, idx),
                (v->size - idx) * v->elem_size);
    }
    memcpy(elem_ptr(v, idx), elem, v->elem_size);
    v->size++;
}

void vec_remove(vector_t* v, size_t idx) {
    assert(v && idx < v->size);
    v->size--;
    if (idx < v->size) {
        memmove(elem_ptr(v, idx),
                elem_ptr(v, idx + 1),
                (v->size - idx) * v->elem_size);
    }
}

size_t vec_size(const vector_t* v)     { return v ? v->size : 0; }
size_t vec_capacity(const vector_t* v) { return v ? v->capacity : 0; }
int    vec_empty(const vector_t* v)    { return !v || v->size == 0; }

void vec_clear(vector_t* v) {
    if (v) v->size = 0;
}
