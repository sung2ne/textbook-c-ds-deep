// filename: qsort_usage.c
// gcc -std=c17 -O2 -o qsort_usage qsort_usage.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 정수 오름차순 */
int cmp_int_asc(const void *a, const void *b) {
    int x = *(int*)a, y = *(int*)b;
    return (x > y) - (x < y);
}

/* 정수 내림차순 */
int cmp_int_desc(const void *a, const void *b) {
    return cmp_int_asc(b, a);  /* 인자 순서만 뒤집으면 됨 */
}

/* double */
int cmp_double(const void *a, const void *b) {
    double x = *(double*)a, y = *(double*)b;
    if (x < y) return -1;
    if (x > y) return  1;
    return 0;
}

/* 문자열 포인터 배열 */
int cmp_str(const void *a, const void *b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

/* 문자열 길이 우선, 같으면 사전순 */
int cmp_str_by_len(const void *a, const void *b) {
    const char *sa = *(const char**)a;
    const char *sb = *(const char**)b;
    size_t la = strlen(sa), lb = strlen(sb);
    if (la != lb) return (la > lb) - (la < lb);
    return strcmp(sa, sb);
}

/* 구조체: 학생 */
typedef struct {
    int  score;
    char name[32];
} Student;

int cmp_student_score_desc(const void *a, const void *b) {
    const Student *sa = a, *sb = b;
    /* 점수 내림차순, 동점이면 이름 오름차순 */
    if (sa->score != sb->score)
        return (sb->score > sa->score) - (sb->score < sa->score);
    return strcmp(sa->name, sb->name);
}

/* qsort_r 예제 (glibc/POSIX 확장) */
#ifdef _GNU_SOURCE
int cmp_student_field(const void *a, const void *b, void *ctx) {
    const Student *sa = a, *sb = b;
    int field = *(int*)ctx;
    if (field == 0) {
        return (sa->score > sb->score) - (sa->score < sb->score);
    }
    return strcmp(sa->name, sb->name);
}
#endif

static void print_int_arr(const int *arr, int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        printf("%d", arr[i]);
        if (i < n-1) printf(", ");
    }
    printf("]\n");
}

int main(void) {
    /* 정수 배열 */
    int nums[] = {5, 3, 8, 1, 9, 2, 7, 4, 6};
    int n = sizeof(nums) / sizeof(nums[0]);

    qsort(nums, (size_t)n, sizeof(int), cmp_int_asc);
    printf("int 오름차순: "); print_int_arr(nums, n);

    qsort(nums, (size_t)n, sizeof(int), cmp_int_desc);
    printf("int 내림차순: "); print_int_arr(nums, n);

    /* 문자열 배열 */
    const char *words[] = {"banana", "apple", "cherry", "date", "fig", "elderberry"};
    int nw = sizeof(words) / sizeof(words[0]);

    qsort(words, (size_t)nw, sizeof(char*), cmp_str);
    printf("\n문자열 사전순: [");
    for (int i = 0; i < nw; i++) printf("%s%s", words[i], i<nw-1?", ":"");
    printf("]\n");

    qsort(words, (size_t)nw, sizeof(char*), cmp_str_by_len);
    printf("문자열 길이순: [");
    for (int i = 0; i < nw; i++) printf("%s%s", words[i], i<nw-1?", ":"");
    printf("]\n");

    /* 구조체 배열 */
    Student students[] = {
        {85, "김민준"}, {92, "이서연"}, {85, "박지호"},
        {78, "최수아"}, {92, "정우진"},
    };
    int ns = sizeof(students) / sizeof(students[0]);

    qsort(students, (size_t)ns, sizeof(Student), cmp_student_score_desc);
    printf("\n학생 점수 내림차순 (동점 시 이름 오름차순):\n");
    for (int i = 0; i < ns; i++)
        printf("  [%d위] %d점 %s\n", i+1, students[i].score, students[i].name);

    /* qsort_r 예제 */
#ifdef _GNU_SOURCE
    int field = 1;  /* 이름 기준 */
    qsort_r(students, (size_t)ns, sizeof(Student), cmp_student_field, &field);
    printf("\n학생 이름 오름차순 (qsort_r):\n");
    for (int i = 0; i < ns; i++)
        printf("  %s (%d)\n", students[i].name, students[i].score);
#endif

    /* comparator 검증 */
    printf("\n=== comparator 검증 ===\n");
    int a = 5, b = 3, c = 7;
    printf("cmp(%d,%d)=%d, cmp(%d,%d)=%d (부호 반대여야 함)\n",
           a, b, cmp_int_asc(&a, &b),
           b, a, cmp_int_asc(&b, &a));
    printf("cmp(%d,%d)=%d (0이어야 함)\n",
           a, a, cmp_int_asc(&a, &a));
    printf("추이성: cmp(%d,%d)=%d, cmp(%d,%d)=%d → cmp(%d,%d)=%d\n",
           b, a, cmp_int_asc(&b, &a),
           a, c, cmp_int_asc(&a, &c),
           b, c, cmp_int_asc(&b, &c));

    return 0;
}
