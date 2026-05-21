// filename: autocomplete.c
// gcc -std=c17 -O2 -o autocomplete autocomplete.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ALPHA_SIZE 26
#define MAX_WORD   128
#define MAX_CAND   256

/* ── 트라이 노드 ── */
typedef struct TrieNode {
    struct TrieNode *children[ALPHA_SIZE];
    int              freq;      /* 이 단어의 검색 빈도 */
    bool             is_end;
} TrieNode;

static TrieNode *new_node(void) {
    return calloc(1, sizeof(TrieNode));
}

/* ── 삽입 (freq 누적) ── */
void ac_insert(TrieNode *root, const char *word, int freq) {
    TrieNode *cur = root;
    for (int i = 0; word[i]; i++) {
        int idx = word[i] - 'a';
        if (!cur->children[idx])
            cur->children[idx] = new_node();
        cur = cur->children[idx];
    }
    cur->is_end  = true;
    cur->freq   += freq;     /* 같은 단어 중복 삽입 허용: 빈도 누적 */
}

/* ── 후보 구조체 ── */
typedef struct {
    char word[MAX_WORD];
    int  freq;
} Candidate;

static Candidate cands[MAX_CAND];
static int       cand_cnt;

/* ── DFS로 후보 수집 ── */
static void collect(TrieNode *node, char *buf, int depth) {
    if (node->is_end && cand_cnt < MAX_CAND) {
        strncpy(cands[cand_cnt].word, buf, MAX_WORD - 1);
        cands[cand_cnt].freq = node->freq;
        cand_cnt++;
    }
    for (int i = 0; i < ALPHA_SIZE; i++) {
        if (node->children[i]) {
            buf[depth]     = 'a' + i;
            buf[depth + 1] = '\0';
            collect(node->children[i], buf, depth + 1);
        }
    }
}

/* ── 빈도 내림차순 비교 ── */
static int cmp_freq(const void *a, const void *b) {
    return ((Candidate *)b)->freq - ((Candidate *)a)->freq;
}

/* ── 자동완성 상위 k개 제안 ── */
void ac_suggest(TrieNode *root, const char *prefix, int k) {
    TrieNode *cur = root;
    for (int i = 0; prefix[i]; i++) {
        int idx = prefix[i] - 'a';
        if (!cur->children[idx]) {
            printf("  (제안 없음)\n");
            return;
        }
        cur = cur->children[idx];
    }

    char buf[MAX_WORD];
    strncpy(buf, prefix, MAX_WORD - 1);
    cand_cnt = 0;
    collect(cur, buf, (int)strlen(prefix));

    qsort(cands, cand_cnt, sizeof(Candidate), cmp_freq);

    int show = cand_cnt < k ? cand_cnt : k;
    for (int i = 0; i < show; i++)
        printf("  %d. %-20s (빈도: %d)\n",
               i + 1, cands[i].word, cands[i].freq);
}

static void trie_free(TrieNode *root) {
    if (!root) return;
    for (int i = 0; i < ALPHA_SIZE; i++)
        trie_free(root->children[i]);
    free(root);
}

int main(void) {
    TrieNode *root = new_node();

    /* 단어와 빈도 */
    struct { const char *word; int freq; } data[] = {
        {"python",     15000},
        {"pytorch",     8000},
        {"pytest",      4000},
        {"pycharm",     3000},
        {"pip",        12000},
        {"pandas",     11000},
        {"paramiko",    1500},
        {"pathlib",     2000},
    };

    for (int i = 0; i < 8; i++)
        ac_insert(root, data[i].word, data[i].freq);

    printf("=== 자동완성: 'py' (상위 3개) ===\n");
    ac_suggest(root, "py", 3);

    printf("\n=== 자동완성: 'pa' (상위 3개) ===\n");
    ac_suggest(root, "pa", 3);

    printf("\n=== 자동완성: 'pi' (상위 3개) ===\n");
    ac_suggest(root, "pi", 3);

    trie_free(root);
    return 0;
}
