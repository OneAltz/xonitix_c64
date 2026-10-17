#ifndef XONITIX_GAME_H
#define XONITIX_GAME_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int pos;
    int direction;
    int speed;
} Actor;

typedef struct {
    int width;               /* Valid positions: 0 through width, inclusive. */
    int level;
    long long score;
    int hero_pos;
    int hero_direction;
    bool firing;
    bool dead;
    unsigned char *walls;    /* width + 1 bytes, one per possible position. */
    int claimed;
    Actor *enemies;
    size_t enemy_count;
} Game;

/* The caller seeds rand() before starting the first level. */
bool game_init(Game *game, int width);
bool game_next_level(Game *game);
void game_free(Game *game);
void game_move(Game *game, int direction); /* -1 = left, 0 = stop, 1 = right */
void game_fire(Game *game);
void game_tick(Game *game);
bool game_level_complete(const Game *game);

#endif
