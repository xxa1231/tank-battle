/*
 * Tank Battle — C Console Game
 * Simple tank battle game using Windows Console API
 * Controls: WASD/Arrow keys to move, Space to shoot, Q to quit
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>

/* ── Constants ─────────────────────────────────────────────────────────── */
#define MAP_W      20
#define MAP_H      15
#define MAX_ENEMIES 4
#define MAX_BULLETS 10
#define PLAYER_HP   3
#define FPS         12

/* Map tiles */
#define TILE_EMPTY  0
#define TILE_BRICK  1
#define TILE_STEEL  2

/* Directions */
#define DIR_UP    0
#define DIR_DOWN  1
#define DIR_LEFT  2
#define DIR_RIGHT 3

/* ── Structures ────────────────────────────────────────────────────────── */
typedef struct {
    int x, y;
    int dir;
    int hp;
    int alive;
    int shoot_cooldown;
} Tank;

typedef struct {
    int x, y;
    int dir;
    int active;
    int is_player;   /* 1 = player bullet, 0 = enemy bullet */
} Bullet;

/* ── Global State ──────────────────────────────────────────────────────── */
int map[MAP_H][MAP_W];
Tank player;
Tank enemies[MAX_ENEMIES];
Bullet bullets[MAX_BULLETS];
int score = 0;
int game_over = 0;
int game_won = 0;
int enemy_count = 0;
HANDLE hConsole;

/* ── Map Layout ────────────────────────────────────────────────────────── */
/* Classic tank battle map: walls around edges, some obstacles in middle */
void InitMap(void) {
    int initial[MAP_H][MAP_W] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,1,0,1,1,0,0,1,1,0,0,1,0,0,0,0,1},
        {1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
        {1,0,2,0,1,0,0,0,0,0,0,0,0,0,1,0,2,0,0,1},
        {1,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
        {1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
        {1,0,2,0,1,0,0,0,0,0,0,0,0,0,1,0,2,0,0,1},
        {1,0,0,0,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    };
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++)
            map[y][x] = initial[y][x];
}

/* ── Rendering ─────────────────────────────────────────────────────────── */
void GotoXY(int x, int y) {
    COORD coord = {x * 2, y};
    SetConsoleCursorPosition(hConsole, coord);
}

void SetColor(int fg, int bg) {
    SetConsoleTextAttribute(hConsole, (bg << 4) | fg);
}

void DrawTile(int x, int y) {
    GotoXY(x, y);
    switch (map[y][x]) {
        case TILE_BRICK: SetColor(6, 0);  printf("▓▓"); break;  /* brown */
        case TILE_STEEL: SetColor(7, 0);  printf("██"); break;  /* white */
        default:                      printf("  "); break;
    }
    SetColor(7, 0);  /* reset to white */
}

void DrawTank(int x, int y, int dir, int is_player) {
    GotoXY(x, y);
    if (is_player)
        SetColor(10, 0);  /* green for player */
    else
        SetColor(12, 0);  /* red for enemies */

    switch (dir) {
        case DIR_UP:    printf("▲▲"); break;
        case DIR_DOWN:  printf("▼▼"); break;
        case DIR_LEFT:  printf("◄◄"); break;
        case DIR_RIGHT: printf("►►"); break;
    }
    SetColor(7, 0);
}

void DrawBullet(int x, int y) {
    GotoXY(x, y);
    SetColor(14, 0);  /* yellow */
    printf("●");
    SetColor(7, 0);
}

void DrawHUD(void) {
    SetColor(11, 0);  /* cyan */
    GotoXY(0, MAP_H + 1);
    printf("Score: %-4d  Lives: ", score);
    SetColor(10, 0);
    for (int i = 0; i < player.hp; i++) printf("♥ ");
    for (int i = player.hp; i < PLAYER_HP; i++) printf("  ");
    SetColor(11, 0);
    printf("  Enemies: %d  ", enemy_count);
    printf("WASD:Move  Space:Shoot  Q:Quit");
    SetColor(7, 0);
}

void RenderAll(void) {
    /* Draw entire map */
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++)
            DrawTile(x, y);

    /* Draw bullets */
    for (int i = 0; i < MAX_BULLETS; i++)
        if (bullets[i].active)
            DrawBullet(bullets[i].x, bullets[i].y);

    /* Draw enemies */
    for (int i = 0; i < MAX_ENEMIES; i++)
        if (enemies[i].alive)
            DrawTank(enemies[i].x, enemies[i].y, enemies[i].dir, 0);

    /* Draw player */
    if (player.alive)
        DrawTank(player.x, player.y, player.dir, 1);

    DrawHUD();
}

/* ── Bullet System ─────────────────────────────────────────────────────── */
Bullet* GetFreeBullet(void) {
    for (int i = 0; i < MAX_BULLETS; i++)
        if (!bullets[i].active)
            return &bullets[i];
    return NULL;
}

void SpawnBullet(int x, int y, int dir, int is_player) {
    Bullet* b = GetFreeBullet();
    if (!b) return;

    b->x = x;
    b->y = y;
    b->dir = dir;
    b->active = 1;
    b->is_player = is_player;

    /* Offset bullet in front of tank */
    switch (dir) {
        case DIR_UP:    b->y -= 1; break;
        case DIR_DOWN:  b->y += 1; break;
        case DIR_LEFT:  b->x -= 1; break;
        case DIR_RIGHT: b->x += 1; break;
    }
}

/* ── Collision Detection ───────────────────────────────────────────────── */
int IsBlocked(int x, int y) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return 1;
    if (map[y][x] != TILE_EMPTY) return 1;
    return 0;
}

int HitTank(int x, int y) {
    /* Check player */
    if (player.alive && player.x == x && player.y == y)
        return -1;  /* player hit */

    /* Check enemies */
    for (int i = 0; i < MAX_ENEMIES; i++)
        if (enemies[i].alive && enemies[i].x == x && enemies[i].y == y)
            return i;  /* enemy index */

    return -2;  /* no tank hit */
}

int HitWall(int x, int y) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return -1;
    if (map[y][x] == TILE_BRICK) return 1;
    if (map[y][x] == TILE_STEEL) return 2;
    return 0;
}

void UpdateBullets(void) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;

        /* Move bullet */
        int new_x = bullets[i].x, new_y = bullets[i].y;
        switch (bullets[i].dir) {
            case DIR_UP:    new_y--; break;
            case DIR_DOWN:  new_y++; break;
            case DIR_LEFT:  new_x--; break;
            case DIR_RIGHT: new_x++; break;
        }

        /* Out of bounds */
        if (new_x < 0 || new_x >= MAP_W || new_y < 0 || new_y >= MAP_H) {
            bullets[i].active = 0;
            continue;
        }

        /* Hit wall */
        int wall = HitWall(new_x, new_y);
        if (wall == 1) {  /* brick: destroy */
            map[new_y][new_x] = TILE_EMPTY;
            bullets[i].active = 0;
            continue;
        }
        if (wall == 2) {  /* steel: bounce back */
            bullets[i].active = 0;
            continue;
        }

        /* Hit tank */
        int tank_hit = HitTank(new_x, new_y);
        if (tank_hit >= 0) {  /* enemy hit */
            enemies[tank_hit].alive = 0;
            enemies[tank_hit].hp = 0;
            enemy_count--;
            score += 100;
            bullets[i].active = 0;
            if (enemy_count == 0) game_won = 1;
            continue;
        }
        if (tank_hit == -1) {  /* player hit */
            player.hp--;
            bullets[i].active = 0;
            if (player.hp <= 0) {
                player.alive = 0;
                game_over = 1;
            }
            continue;
        }

        /* Move to new position */
        bullets[i].x = new_x;
        bullets[i].y = new_y;
    }
}

/* ── Tank Movement ─────────────────────────────────────────────────────── */
int CanMove(int x, int y) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return 0;
    if (map[y][x] != TILE_EMPTY) return 0;
    /* Check no other tank at same position */
    if (player.alive && player.x == x && player.y == y) return 0;
    for (int i = 0; i < MAX_ENEMIES; i++)
        if (enemies[i].alive && enemies[i].x == x && enemies[i].y == y)
            return 0;
    return 1;
}

/* ── Input ─────────────────────────────────────────────────────────────── */
void HandleInput(void) {
    int moved = 0;
    int dx = 0, dy = 0;

    /* Movement: WASD + Arrows */
    if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState(VK_UP)    & 0x8000) { dy = -1; player.dir = DIR_UP;    moved = 1; }
    if (GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState(VK_DOWN)  & 0x8000) { dy =  1; player.dir = DIR_DOWN;  moved = 1; }
    if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState(VK_LEFT)  & 0x8000) { dx = -1; player.dir = DIR_LEFT;  moved = 1; }
    if (GetAsyncKeyState('D') & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000) { dx =  1; player.dir = DIR_RIGHT; moved = 1; }

    if (moved && CanMove(player.x + dx, player.y + dy)) {
        player.x += dx;
        player.y += dy;
    }

    /* Shooting */
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        if (player.shoot_cooldown <= 0) {
            SpawnBullet(player.x, player.y, player.dir, 1);
            player.shoot_cooldown = 5;  /* cooldown frames */
        }
    }

    /* Quit */
    if (GetAsyncKeyState('Q') & 0x8000)
        game_over = 1;

    if (player.shoot_cooldown > 0) player.shoot_cooldown--;
}

/* ── Enemy AI ──────────────────────────────────────────────────────────── */
void EnemyAI(void) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;

        /* Randomly change direction (sometimes) */
        if (rand() % 20 == 0)
            enemies[i].dir = rand() % 4;

        /* Move in current direction */
        int dx = 0, dy = 0;
        switch (enemies[i].dir) {
            case DIR_UP:    dy = -1; break;
            case DIR_DOWN:  dy =  1; break;
            case DIR_LEFT:  dx = -1; break;
            case DIR_RIGHT: dx =  1; break;
        }

        int nx = enemies[i].x + dx;
        int ny = enemies[i].y + dy;

        if (CanMove(nx, ny)) {
            enemies[i].x = nx;
            enemies[i].y = ny;
        } else {
            /* Hit something, pick new direction */
            enemies[i].dir = rand() % 4;
        }

        /* Randomly shoot toward player direction */
        if (rand() % 15 == 0) {
            int aim_dir;
            int px = player.x, py = player.y;

            /* Simple aiming: prefer axis toward player */
            if (abs(px - enemies[i].x) > abs(py - enemies[i].y))
                aim_dir = (px > enemies[i].x) ? DIR_RIGHT : DIR_LEFT;
            else
                aim_dir = (py > enemies[i].y) ? DIR_DOWN : DIR_UP;

            SpawnBullet(enemies[i].x, enemies[i].y, aim_dir, 0);
        }
    }
}

/* ── Init ──────────────────────────────────────────────────────────────── */
void InitGame(void) {
    srand((unsigned)time(NULL));

    InitMap();

    /* Player */
    player.x = MAP_W / 2;
    player.y = MAP_H - 2;
    player.dir = DIR_UP;
    player.hp = PLAYER_HP;
    player.alive = 1;
    player.shoot_cooldown = 0;

    /* Enemies: spawn at top */
    enemy_count = MAX_ENEMIES;
    int spawn_positions[4][2] = {{2,1}, {MAP_W/2,1}, {MAP_W-3,1}, {MAP_W/2,3}};
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].x = spawn_positions[i][0];
        enemies[i].y = spawn_positions[i][1];
        enemies[i].dir = DIR_DOWN;
        enemies[i].hp = 1;
        enemies[i].alive = 1;
        enemies[i].shoot_cooldown = 0;
    }

    /* Bullets */
    for (int i = 0; i < MAX_BULLETS; i++)
        bullets[i].active = 0;

    score = 0;
    game_over = 0;
    game_won = 0;
}

/* ── Screens ───────────────────────────────────────────────────────────── */
void ShowTitleScreen(void) {
    system("cls");
    SetColor(14, 0);
    printf("\n\n\n");
    printf("         ╔══════════════════════════════════╗\n");
    printf("         ║          TANK  BATTLE            ║\n");
    printf("         ║         C Console Edition        ║\n");
    printf("         ╠══════════════════════════════════╣\n");
    printf("         ║  ▲▲  WASD / Arrows : Move       ║\n");
    printf("         ║   ●  Space         : Shoot      ║\n");
    printf("         ║      Q             : Quit       ║\n");
    printf("         ╠══════════════════════════════════╣\n");
    printf("         ║  Destroy all enemy tanks to win! ║\n");
    printf("         ║  You have 3 lives. Good luck!    ║\n");
    printf("         ╚══════════════════════════════════╝\n");
    printf("\n");
    SetColor(10, 0);
    printf("            Press any key to start...");
    SetColor(7, 0);
    _getch();
}

void ShowGameOverScreen(void) {
    system("cls");
    SetColor(12, 0);
    printf("\n\n\n");
    printf("         ╔══════════════════════════════════╗\n");
    printf("         ║           GAME  OVER             ║\n");
    printf("         ╠══════════════════════════════════╣\n");
    printf("         ║                                 ║\n");
    SetColor(14, 0);
    printf("         ║  Final Score: %-6d            ║\n", score);
    SetColor(12, 0);
    printf("         ║                                 ║\n");
    printf("         ╚══════════════════════════════════╝\n");
    printf("\n");
    SetColor(7, 0);
    printf("            Press R to restart, Q to quit...");
}

void ShowWinScreen(void) {
    system("cls");
    SetColor(10, 0);
    printf("\n\n\n");
    printf("         ╔══════════════════════════════════╗\n");
    printf("         ║          YOU WIN! 🎉             ║\n");
    printf("         ╠══════════════════════════════════╣\n");
    printf("         ║                                 ║\n");
    SetColor(14, 0);
    printf("         ║  Final Score: %-6d            ║\n", score);
    SetColor(10, 0);
    printf("         ║  All enemies destroyed!         ║\n");
    printf("         ║                                 ║\n");
    printf("         ╚══════════════════════════════════╝\n");
    printf("\n");
    SetColor(7, 0);
    printf("            Press R to restart, Q to quit...");
}

int WaitForRestart(void) {
    while (1) {
        if (_kbhit()) {
            char c = _getch();
            if (c == 'r' || c == 'R') return 1;
            if (c == 'q' || c == 'Q') return 0;
        }
        Sleep(50);
    }
}

/* ── Main ──────────────────────────────────────────────────────────────── */
int main(void) {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    /* Hide cursor */
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    /* Set console window title */
    SetConsoleTitle("Tank Battle - C Console Edition");

    ShowTitleScreen();

    do {
        system("cls");
        InitGame();
        RenderAll();

        /* Main game loop */
        while (!game_over && !game_won) {
            HandleInput();
            EnemyAI();
            UpdateBullets();
            RenderAll();
            Sleep(1000 / FPS);
        }

        /* Show result screen */
        if (game_won)
            ShowWinScreen();
        else if (player.hp <= 0)
            ShowGameOverScreen();
        else
            break;  /* Q pressed */

    } while (WaitForRestart());

    system("cls");
    SetColor(7, 0);
    printf("\nThanks for playing Tank Battle!\n\n");
    return 0;
}
