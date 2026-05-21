// filename: trie.c
// gcc -std=c17 -O2 -o trie trie.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ALPHA_SIZE 26

/* ── 트라이 노드 ── */
typedef struct TrieNode {
    struct TrieNode *children[ALPHA_SIZE];
    bool             is_end;    /* 단어 끝 표시 */
    int              count;     /* 이 prefix로 시작하는 단어 수 */
} TrieNode;

static TrieNode *new_node(void) {
    TrieNode *n = calloc(1, sizeof(TrieNode));
    return n;
}

/* ── 삽입 ── */
void trie_insert(TrieNode *root, const char *word) {
    TrieNode *cur = root;
    for (int i = 0; word[i] != '\0'; i++) {
        int idx = word[i] - 'a';
        if (!cur->children[idx])
            cur->children[idx] = new_node();
        cur = cur->children[idx];
        cur->count++;             /* 이 prefix를 공유하는 단어 수 */
    }
    cur->is_end = true;
}

/* ── 탐색 ── */
bool trie_search(TrieNode *root, const char *word) {
    TrieNode *cur = root;
    for (int i = 0; word[i] != '\0'; i++) {
        int idx = word[i] - 'a';
        if (!cur->children[idx]) return false;
        cur = cur->children[idx];
    }
    return cur->is_end;
}

/* ── prefix 탐색 — 해당 prefix가 존재하는가 ── */
bool trie_starts_with(TrieNode *root, const char *prefix) {
    TrieNode *cur = root;
    for (int i = 0; prefix[i] != '\0'; i++) {
        int idx = prefix[i] - 'a';
        if (!cur->children[idx]) return false;
        cur = cur->children[idx];
    }
    return true;
}

/* ── 자동완성: prefix에 해당하는 단어 출력 ── */
static void autocomplete_dfs(TrieNode *node, char *buf, int depth) {
    if (node->is_end)
        printf("  → %s\n", buf);

    for (int i = 0; i < ALPHA_SIZE; i++) {
        if (node->children[i]) {
            buf[depth]     = 'a' + i;
            buf[depth + 1] = '\0';
            autocomplete_dfs(node->children[i], buf, depth + 1);
        }
    }
}

void trie_autocomplete(TrieNode *root, const char *prefix) {
    TrieNode *cur = root;
    for (int i = 0; prefix[i] != '\0'; i++) {
        int idx = prefix[i] - 'a';
        if (!cur->children[idx]) {
            printf("  (없음)\n");
            return;
        }
        cur = cur->children[idx];
    }
    char buf[256];
    strncpy(buf, prefix, sizeof(buf));
    autocomplete_dfs(cur, buf, (int)strlen(prefix));
}

/* ── 메모리 해제 ── */
void trie_free(TrieNode *root) {
    if (!root) return;
    for (int i = 0; i < ALPHA_SIZE; i++)
        trie_free(root->children[i]);
    free(root);
}

int main(void) {
    TrieNode *root = new_node();

    const char *words[] = {
        "python", "pytorch", "pytest", "pycharm",
        "java", "javascript", "javac",
        "rust", "rustup"
    };
    for (int i = 0; i < 9; i++)
        trie_insert(root, words[i]);

    printf("탐색 'python' : %s\n", trie_search(root, "python") ? "있음" : "없음");
    printf("탐색 'pypy'   : %s\n", trie_search(root, "pypy")   ? "있음" : "없음");

    printf("\n자동완성 'py':\n");
    trie_autocomplete(root, "py");

    printf("\n자동완성 'java':\n");
    trie_autocomplete(root, "java");

    trie_free(root);
    return 0;
}
