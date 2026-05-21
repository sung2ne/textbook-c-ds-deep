// filename: oop_style.c
// 전형적인 OOP 스타일 구조체
typedef struct {
    float   pos_x, pos_y, pos_z;  /* 위치 12바이트 */
    float   vel_x, vel_y, vel_z;  /* 속도 12바이트 */
    int     health;                /* 체력 4바이트 */
    int     armor;                 /* 방어력 4바이트 */
    char    name[32];              /* 이름 32바이트 */
    /* ... 기타 필드들 */
} Enemy;                           /* 총 ~64바이트 이상 */

Enemy enemies[1000];
