// filename: function_pointers.c
// gcc -std=c17 -O2 -o function_pointers function_pointers.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* 기본 함수 포인터 */
typedef int (*binary_op_fn)(int, int);

int op_add(int a, int b) { return a + b; }
int op_sub(int a, int b) { return a - b; }
int op_mul(int a, int b) { return a * b; }

/* dispatch table */
typedef struct {
    const char*  name;
    binary_op_fn fn;
} OpEntry;

static const OpEntry op_table[] = {
    { "add", op_add },
    { "sub", op_sub },
    { "mul", op_mul },
};

binary_op_fn find_op(const char* name) {
    for (size_t i = 0; i < sizeof(op_table) / sizeof(op_table[0]); i++) {
        if (strcmp(op_table[i].name, name) == 0) {
            return op_table[i].fn;
        }
    }
    return NULL;
}

/* qsort 비교 함수 */
int cmp_int_asc(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);   /* 오버플로우 없는 비교 관용구 */
}

/* OOP 패턴 */
typedef struct Shape Shape;
struct Shape {
    const char* name;
    double (*area)(const Shape*);
};

typedef struct { Shape base; double r; } Circle;
typedef struct { Shape base; double w, h; } Rect;

double circle_area(const Shape* s) {
    return M_PI * ((Circle*)s)->r * ((Circle*)s)->r;
}
double rect_area(const Shape* s) {
    return ((Rect*)s)->w * ((Rect*)s)->h;
}

int main(void) {
    /* dispatch table */
    printf("=== Dispatch Table ===\n");
    const char* ops[] = { "add", "sub", "mul", "div" };
    for (size_t i = 0; i < 4; i++) {
        binary_op_fn op = find_op(ops[i]);
        if (op) printf("  %s(10, 3) = %d\n", ops[i], op(10, 3));
        else    printf("  %s: 없음\n", ops[i]);
    }

    /* qsort */
    printf("\n=== qsort ===\n");
    int arr[] = {5, 2, 8, 1, 9, 3};
    int n = (int)(sizeof(arr) / sizeof(arr[0]));
    qsort(arr, (size_t)n, sizeof(int), cmp_int_asc);
    printf("  정렬 결과: ");
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");

    /* OOP 패턴 */
    printf("\n=== OOP 패턴 ===\n");
    Circle c = { { "Circle", circle_area }, 5.0 };
    Rect   r = { { "Rect",   rect_area   }, 4.0, 6.0 };

    const Shape* shapes[] = { (Shape*)&c, (Shape*)&r };
    for (int i = 0; i < 2; i++) {
        printf("  %s area = %.2f\n", shapes[i]->name, shapes[i]->area(shapes[i]));
    }

    return 0;
}
