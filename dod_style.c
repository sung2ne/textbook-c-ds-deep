// filename: dod_style.c
// DOD 스타일: 자주 함께 처리되는 데이터를 묶음
#define MAX_ENEMIES 1000

/* 물리 데이터만 모은 배열 */
float pos_x[MAX_ENEMIES];
float pos_y[MAX_ENEMIES];
float pos_z[MAX_ENEMIES];
float vel_x[MAX_ENEMIES];
float vel_y[MAX_ENEMIES];
float vel_z[MAX_ENEMIES];

/* 게임플레이 데이터 (물리 루프에서는 건드리지 않음) */
int   health[MAX_ENEMIES];
int   armor[MAX_ENEMIES];
char  name[MAX_ENEMIES][32];

void update_physics_dod(int n, float dt) {
    for (int i = 0; i < n; i++) {
        pos_x[i] += vel_x[i] * dt;
        pos_y[i] += vel_y[i] * dt;
        pos_z[i] += vel_z[i] * dt;
    }
}
