// filename: maze_bfs.c
// gcc -std=c17 -O2 -o maze maze_bfs.c

#include <stdio.h>
#include <string.h>

#define ROWS 10
#define COLS 12

/* 0=빈칸, 1=벽 */
int maze[ROWS][COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,1,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,1,0,0,0,1},
    {1,1,1,1,1,0,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1},
};

typedef struct { int r, c; } Pos;

Pos parent_pos[ROWS][COLS];
int dist_maze[ROWS][COLS];
int visited_maze[ROWS][COLS];

int dr[] = {-1, 1, 0, 0};
int dc[] = {0, 0, -1, 1};

int bfs_maze(Pos start, Pos end) {
    Pos queue[ROWS * COLS];
    int front = 0, back = 0;

    memset(dist_maze, 0x3f, sizeof(dist_maze));
    memset(visited_maze, 0, sizeof(visited_maze));

    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            parent_pos[i][j] = (Pos){-1, -1};

    dist_maze[start.r][start.c] = 0;
    visited_maze[start.r][start.c] = 1;
    queue[back++] = start;

    while (front < back) {
        Pos cur = queue[front++];

        if (cur.r == end.r && cur.c == end.c)
            return dist_maze[end.r][end.c];

        for (int d = 0; d < 4; d++) {
            int nr = cur.r + dr[d];
            int nc = cur.c + dc[d];
            if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
            if (maze[nr][nc] == 1 || visited_maze[nr][nc]) continue;

            visited_maze[nr][nc] = 1;
            dist_maze[nr][nc] = dist_maze[cur.r][cur.c] + 1;
            parent_pos[nr][nc] = cur;
            queue[back++] = (Pos){nr, nc};
        }
    }
    return -1;  /* 도달 불가 */
}

void print_maze_with_path(Pos start, Pos end) {
    /* 경로를 역추적해서 마킹 */
    char display[ROWS][COLS + 1];
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++)
            display[i][j] = maze[i][j] ? '#' : ' ';
        display[i][COLS] = '\0';
    }
    display[start.r][start.c] = 'S';
    display[end.r][end.c]     = 'E';

    /* 경로 표시 */
    Pos cur = end;
    while (parent_pos[cur.r][cur.c].r != -1) {
        Pos p = parent_pos[cur.r][cur.c];
        if (!(cur.r == start.r && cur.c == start.c))
            if (!(cur.r == end.r && cur.c == end.c))
                display[cur.r][cur.c] = '.';
        cur = p;
    }

    printf("\n=== 미로 최단 경로 ===\n");
    for (int i = 0; i < ROWS; i++)
        printf("%s\n", display[i]);
}

int main(void) {
    Pos start = {1, 1};
    Pos end   = {8, 10};

    int dist = bfs_maze(start, end);
    if (dist == -1)
        printf("경로 없음\n");
    else {
        printf("최단 거리: %d\n", dist);
        print_maze_with_path(start, end);
    }
    return 0;
}
