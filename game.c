/* Xonitix C11 port. Original game copyright 2018 Breno G. de Oliveira.
 * Released under the MIT License; see LICENSE. */
#include "game.h"

#include <stdlib.h>
#include <string.h>

bool game_init(Game *game, int width) {
    if (!game || width < 2 || width > 4096) {
        return false;
    }
    memset(game, 0, sizeof(*game));
    game->walls = calloc((size_t)width + 1, sizeof(*game->walls));
    if (!game->walls) {
        return false;
    }
    game->width = width;
    return true;
}

void game_free(Game *game) {
    if (!game) {
        return;
    }
    free(game->enemies);
    free(game->walls);
    memset(game, 0, sizeof(*game));
}

bool game_next_level(Game *game) {
    Actor *enemies;
    size_t count;
    if (!game || !game->walls || game->level >= 4096) {
        return false;
    }
    count = (size_t)game->level + 1;
    enemies = realloc(game->enemies, count * sizeof(*enemies));
    if (!enemies) {
        return false;
    }
    game->enemies = enemies;
    game->enemy_count = count;
    game->level++;
    game->hero_pos = rand() % game->width;
    game->hero_direction = 0;
    game->firing = false;
    game->dead = false;
    game->claimed = 0;
    memset(game->walls, 0, (size_t)game->width + 1);
    for (size_t i = 0; i < count; ++i) {
        game->enemies[i].pos = rand() % game->width;
        game->enemies[i].direction = (rand() % 2) ? 1 : -1;
        game->enemies[i].speed = 1 + rand() % 2;
    }
    return true;
}

void game_move(Game *game, int direction) {
    if (game && !game->dead && direction >= -1 && direction <= 1) {
        game->hero_direction = direction;
    }
}

void game_fire(Game *game) {
    if (game && !game->dead) {
        game->firing = true;
    }
}

/* Claim the space between the nearest walls, stopping the fill at the hero
 * on sides containing enemies. Shooting within one cell of an enemy is fatal. */
static void resolve_shot(Game *game) {
    int left = 0;
    int right = game->width;
    const int position = game->hero_pos;

    if (game->walls[position]) {
        return;
    }
    for (int i = 0; i <= game->width; ++i) {
        if (!game->walls[i]) {
            continue;
        }
        if (i < position && i > left) {
            left = i;
        } else if (i > position && i < right) {
            right = i;
        }
    }

    /* Check close shots first, so enemy ordering cannot hide a collision. */
    for (size_t i = 0; i < game->enemy_count; ++i) {
        int x = game->enemies[i].pos;
        if (x >= position - 1 && x <= position + 1) {
            game->dead = true;
            return;
        }
    }
    for (size_t i = 0; i < game->enemy_count; ++i) {
        int x = game->enemies[i].pos;
        if (x > left && x < position - 1) {
            left = position;
        } else if (x < right && x > position + 1) {
            right = position;
        }
    }
    for (int i = left; i <= right; ++i) {
        if (!game->walls[i]) {
            game->walls[i] = 1;
            game->claimed++;
            game->score += 100;
        }
    }
}

static void move_enemy(const Game *game, Actor *enemy) {
    int boundary = enemy->direction == -1 ? 0 : game->width;
    for (int i = 0; i <= game->width; ++i) {
        if (!game->walls[i]) {
            continue;
        }
        if (enemy->direction == -1 && i < enemy->pos && i > boundary) {
            boundary = i;
        } else if (enemy->direction == 1 && i > enemy->pos && i < boundary) {
            boundary = i;
        }
    }
    enemy->pos += enemy->direction * enemy->speed;
    if (enemy->direction == 1 && enemy->pos >= boundary) {
        enemy->pos = boundary - 1;
        enemy->direction = -1;
    } else if (enemy->direction == -1 && enemy->pos <= boundary) {
        enemy->pos = boundary + 1;
        enemy->direction = 1;
    }
    if (enemy->pos < 0) {
        enemy->pos = 0;
    } else if (enemy->pos > game->width) {
        enemy->pos = game->width;
    }
}

void game_tick(Game *game) {
    if (!game || game->dead || !game->walls) {
        return;
    }
    game->hero_pos += game->hero_direction;
    if (game->hero_pos < 0) {
        game->hero_pos = 0;
        game->hero_direction = 0;
    } else if (game->hero_pos > game->width) {
        game->hero_pos = game->width;
        game->hero_direction = 0;
    }
    if (game->firing) {
        game->firing = false;
        resolve_shot(game);
    }
    if (!game->dead) {
        for (size_t i = 0; i < game->enemy_count; ++i) {
            move_enemy(game, &game->enemies[i]);
        }
    }
}

bool game_level_complete(const Game *game) {
    return game && game->width > 0 &&
           ((double)game->claimed / (double)game->width) > 0.50;
}
