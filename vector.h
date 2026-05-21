// filename: vector.h

#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

typedef struct {
    void*  data;
    size_t elem_size;
    size_t size;
    size_t capacity;
} vector_t;

vector_t* vec_create(size_t elem_size, size_t initial_cap);
void      vec_destroy(vector_t* v);

void  vec_push(vector_t* v, const void* elem);
void  vec_pop(vector_t* v, void* out);      /* out: 꺼낸 원소를 저장할 버퍼 */
void* vec_get(const vector_t* v, size_t idx);
void  vec_set(vector_t* v, size_t idx, const void* elem);
void  vec_insert(vector_t* v, size_t idx, const void* elem);
void  vec_remove(vector_t* v, size_t idx);

/* 유틸리티 */
size_t vec_size(const vector_t* v);
size_t vec_capacity(const vector_t* v);
int    vec_empty(const vector_t* v);
void   vec_clear(vector_t* v);

#endif /* VECTOR_H */
