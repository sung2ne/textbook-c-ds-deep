// filename: vector_demo.c
// gcc -std=c17 -O2 -o vector_demo vector_demo.c vector.c

#include "vector.h"
#include <stdio.h>

int main(void) {
    /* int 동적 배열 */
    vector_t* v = vec_create(sizeof(int), 4);

    for (int i = 0; i < 10; i++) {
        vec_push(v, &i);
        printf("push %d → size=%zu, cap=%zu\n",
               i, vec_size(v), vec_capacity(v));
    }

    /* 인덱스 접근 */
    int* elem = vec_get(v, 5);
    printf("v[5] = %d\n", *elem);

    /* 중간 삽입 */
    int val = 99;
    vec_insert(v, 3, &val);
    printf("인덱스 3에 99 삽입 후 v[3] = %d\n",
           *(int*)vec_get(v, 3));

    /* 중간 삭제 */
    vec_remove(v, 0);
    printf("인덱스 0 삭제 후 v[0] = %d\n",
           *(int*)vec_get(v, 0));

    /* pop */
    int out;
    vec_pop(v, &out);
    printf("pop: %d\n", out);

    /* double 동적 배열 */
    vector_t* dv = vec_create(sizeof(double), 2);
    double pi = 3.14159;
    double e  = 2.71828;
    vec_push(dv, &pi);
    vec_push(dv, &e);
    printf("dv[0]=%.5f, dv[1]=%.5f\n",
           *(double*)vec_get(dv, 0),
           *(double*)vec_get(dv, 1));

    vec_destroy(v);
    vec_destroy(dv);
    return 0;
}
