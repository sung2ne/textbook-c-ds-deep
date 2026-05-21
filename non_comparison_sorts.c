// filename: non_comparison_sorts.c
// gcc -std=c17 -O2 -o non_comparison_sorts non_comparison_sorts.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---- Counting Sort ---- */
/* arr의 값이 [0, max_val] 범위 정수일 때 */
void counting_sort(int *arr, int n, int max_val) {
    if (n <= 1 || max_val < 0) return;

    int *count = calloc((size_t)(max_val + 1), sizeof(int));
    int *output = malloc(sizeof(int) * (size_t)n);
    if (!count || !output) { perror("malloc"); exit(1); }

    /* 1단계: count */
    for (int i = 0; i < n; i++) count[arr[i]]++;

    /* 2단계: prefix sum (누적합) */
    for (int i = 1; i <= max_val; i++) count[i] += count[i-1];

    /* 3단계: 오른쪽 → 왼쪽 배치 (안정 정렬) */
    for (int i = n - 1; i >= 0; i--) {
        output[--count[arr[i]]] = arr[i];
    }

    memcpy(arr, output, sizeof(int) * (size_t)n);
    free(count);
    free(output);
}

/* ---- Radix Sort (LSD, 기수=256, 4바이트 정수) ---- */
static void counting_sort_radix(int *arr, int *output, int n, int exp) {
    /* 256개 버킷 (1바이트씩 처리) */
    int count[256] = {0};

    /* 현재 자리의 숫자 추출 */
    for (int i = 0; i < n; i++) {
        int digit = ((unsigned int)arr[i] >> exp) & 0xFF;
        count[digit]++;
    }

    /* prefix sum */
    for (int i = 1; i < 256; i++) count[i] += count[i-1];

    /* 안정 배치 (오른쪽 → 왼쪽) */
    for (int i = n-1; i >= 0; i--) {
        int digit = ((unsigned int)arr[i] >> exp) & 0xFF;
        output[--count[digit]] = arr[i];
    }
}

/* 비음수 정수 Radix Sort (32비트, 4 pass) */
void radix_sort_unsigned(int *arr, int n) {
    int *output = malloc(sizeof(int) * (size_t)n);
    if (!output) { perror("malloc"); exit(1); }

    /* 4바이트 = 4 pass (8비트씩) */
    for (int exp = 0; exp < 32; exp += 8) {
        counting_sort_radix(arr, output, n, exp);
        memcpy(arr, output, sizeof(int) * (size_t)n);
    }
    free(output);
}

/* 음수 지원 Radix Sort:
   음수를 부호 비트 flip으로 양수로 변환 후 정렬, 다시 flip */
void radix_sort(int *arr, int n) {
    /* 부호 비트 flip: 음수를 양수 범위로 이동 */
    for (int i = 0; i < n; i++)
        arr[i] ^= (int)0x80000000;  /* 최상위 비트 toggle */

    radix_sort_unsigned(arr, n);

    /* 복원 */
    for (int i = 0; i < n; i++)
        arr[i] ^= (int)0x80000000;
}

/* ---- Bucket Sort (0.0 ~ 1.0 균등 분포) ---- */
typedef struct BucketNode {
    double            val;
    struct BucketNode *next;
} BucketNode;

static void insertion_sort_bucket(BucketNode **head) {
    if (!*head || !(*head)->next) return;
    BucketNode *sorted = NULL;
    BucketNode *cur = *head;
    while (cur) {
        BucketNode *next = cur->next;
        if (!sorted || cur->val <= sorted->val) {
            cur->next = sorted;
            sorted = cur;
        } else {
            BucketNode *p = sorted;
            while (p->next && p->next->val < cur->val) p = p->next;
            cur->next = p->next;
            p->next = cur;
        }
        cur = next;
    }
    *head = sorted;
}

void bucket_sort(double *arr, int n) {
    BucketNode **buckets = calloc((size_t)n, sizeof(BucketNode*));
    BucketNode *pool = malloc(sizeof(BucketNode) * (size_t)n);
    if (!buckets || !pool) { perror("malloc"); exit(1); }

    /* 분배 */
    for (int i = 0; i < n; i++) {
        int idx = (int)(arr[i] * n);
        if (idx >= n) idx = n-1;  /* 1.0 처리 */
        pool[i].val = arr[i];
        pool[i].next = buckets[idx];
        buckets[idx] = &pool[i];
    }

    /* 각 버킷 정렬 후 수집 */
    int k = 0;
    for (int i = 0; i < n; i++) {
        insertion_sort_bucket(&buckets[i]);
        for (BucketNode *p = buckets[i]; p; p = p->next)
            arr[k++] = p->val;
    }

    free(buckets);
    free(pool);
}

/* ---- 성능 비교 ---- */
static long benchmark_int(void (*fn)(int*, int), const int *orig, int n) {
    int *arr = malloc(sizeof(int) * (size_t)n);
    memcpy(arr, orig, sizeof(int) * (size_t)n);
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    fn(arr, n);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    free(arr);
    return (t1.tv_sec-t0.tv_sec)*1000000L + (t1.tv_nsec-t0.tv_nsec)/1000;
}

static int cmp_int(const void *a, const void *b) {
    int x = *(int*)a, y = *(int*)b;
    return (x>y)-(x<y);
}
static void std_qsort(int *arr, int n) {
    qsort(arr, (size_t)n, sizeof(int), cmp_int);
}

/* counting_sort wrapper: max_val을 캡처 */
static int g_max_val;
static void counting_sort_wrap(int *arr, int n) {
    counting_sort(arr, n, g_max_val);
}

int main(void) {
    /* Counting Sort */
    printf("=== Counting Sort ===\n");
    int cs[] = {4, 1, 3, 1, 2, 4, 3, 0, 2, 4};
    int ncs = 10;
    counting_sort(cs, ncs, 4);
    printf("결과: [");
    for (int i = 0; i < ncs; i++) printf("%d%s", cs[i], i<ncs-1?", ":"");
    printf("]\n");

    /* Radix Sort */
    printf("\n=== Radix Sort ===\n");
    int rs[] = {170, 45, 75, 90, 802, 24, 2, 66, -5, -100, 0};
    int nrs = sizeof(rs)/sizeof(rs[0]);
    radix_sort(rs, nrs);
    printf("결과 (음수 포함): [");
    for (int i = 0; i < nrs; i++) printf("%d%s", rs[i], i<nrs-1?", ":"");
    printf("]\n");

    /* Bucket Sort */
    printf("\n=== Bucket Sort ===\n");
    srand(42);
    double bs[10];
    for (int i = 0; i < 10; i++) bs[i] = (double)rand() / RAND_MAX;
    bucket_sort(bs, 10);
    printf("결과: [");
    for (int i = 0; i < 10; i++) printf("%.3f%s", bs[i], i<9?", ":"");
    printf("]\n");

    /* 성능 비교: Radix vs qsort */
    printf("\n=== Radix Sort vs qsort 성능 비교 ===\n");
    const int N = 1000000;
    int *arr = malloc(sizeof(int) * N);
    srand(42);

    /* 비음수 정수 */
    for (int i = 0; i < N; i++) arr[i] = rand();
    printf("n=%d, 비음수 랜덤 정수:\n", N);
    printf("  qsort:      %ld μs\n", benchmark_int(std_qsort,           arr, N));
    printf("  radix_sort: %ld μs\n", benchmark_int(radix_sort_unsigned, arr, N));

    /* 범위 [0, 999] 정수 (Counting Sort 최적) */
    for (int i = 0; i < N; i++) arr[i] = rand() % 1000;
    g_max_val = 999;
    printf("n=%d, 범위 [0,999] 정수:\n", N);
    printf("  qsort:         %ld μs\n", benchmark_int(std_qsort,          arr, N));
    printf("  counting_sort: %ld μs\n", benchmark_int(counting_sort_wrap, arr, N));
    printf("  radix_sort:    %ld μs\n", benchmark_int(radix_sort_unsigned, arr, N));

    free(arr);

    /* IP 주소 정렬 시뮬레이션 */
    printf("\n=== IP 주소 정렬 (Radix Sort) ===\n");
    /* IP를 uint32로 표현 */
    unsigned int ips[] = {
        (192u<<24)|(168u<<16)|(1u<<8)|100u,
        (10u <<24)|(0u  <<16)|(0u  <<8)|1u,
        (192u<<24)|(168u<<16)|(1u<<8)|1u,
        (172u<<24)|(16u <<16)|(0u  <<8)|5u,
    };
    int nips = sizeof(ips)/sizeof(ips[0]);
    radix_sort_unsigned((int*)ips, nips);
    printf("정렬된 IP 주소:\n");
    for (int i = 0; i < nips; i++) {
        unsigned int ip = ips[i];
        printf("  %u.%u.%u.%u\n",
               (ip>>24)&0xFF, (ip>>16)&0xFF,
               (ip>>8) &0xFF, ip&0xFF);
    }

    return 0;
}
