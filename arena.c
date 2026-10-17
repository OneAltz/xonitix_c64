/* Xonitix Halloween: 2D territory capture and periodic Ghost Mode.
 * Based on Xonitix by Breno G. de Oliveira (2018), MIT. */
#include "arena.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define TICKS_PER_GHOST_WARNING 20  /* 2 seconds of ASCII art */
#define TICKS_PER_AMBIENT_SCARE 12  /* 1.2 seconds of ASCII art */

static int index_of(const Arena *a, int x, int y) {
    return y * a->width + x;
}

static bool inside(const Arena *a, int x, int y) {
    return x >= 0 && x < a->width && y >= 0 && y < a->height;
}

Tile arena_tile(const Arena *a, int x, int y) {
    if (!a || !a->tiles || !inside(a, x, y)) return TILE_CLAIMED;
    return (Tile)a->tiles[index_of(a, x, y)];
}

static void set_tile(Arena *a, int x, int y, Tile tile) {
    a->tiles[index_of(a, x, y)] = (unsigned char)tile;
}

bool arena_init(Arena *a, int width, int height) {
    size_t count;
    if (!a || width < ARENA_MIN_WIDTH || width > ARENA_MAX_WIDTH ||
        height < ARENA_MIN_HEIGHT || height > ARENA_MAX_HEIGHT) return false;
    memset(a, 0, sizeof(*a));
    count = (size_t)width * (size_t)height;
    a->tiles = calloc(count, 1);
    a->visited = calloc(count, 1);
    a->queue = malloc(count * sizeof(*a->queue));
    if (!a->tiles || !a->visited || !a->queue) {
        arena_free(a);
        return false;
    }
    a->width = width;
    a->height = height;
    a->interior_total = (width - 2) * (height - 2);
    a->ghost_enemy = -1;
    a->ghost_interval = 200; /* 20 seconds before first ghost, then between ghosts */
    a->ghost_duration = 50;  /* 5 seconds */
    a->ghost_cooldown = a->ghost_interval;
    a->ambient_cooldown = 95 + rand() % 55;
    a->ascii_enabled = true;
    a->ghosts_enabled = true;
    return true;
}

void arena_free(Arena *a) {
    if (!a) return;
    free(a->tiles);
    free(a->visited);
    free(a->queue);
    memset(a, 0, sizeof(*a));
}

static bool enemy_occupies(const Arena *a, int prior, int x, int y) {
    for (int n = 0; n < prior; ++n) {
        if (a->enemies[n].x == x && a->enemies[n].y == y) return true;
    }
    return false;
}

bool arena_next_level(Arena *a) {
    if (!a || !a->tiles || a->level == INT_MAX) return false;
    a->level++;
    a->enemy_count = a->level + 1;
    if (a->enemy_count > ARENA_MAX_ENEMIES) a->enemy_count = ARENA_MAX_ENEMIES;
    memset(a->tiles, TILE_EMPTY, (size_t)a->width * (size_t)a->height);
    for (int y = 0; y < a->height; ++y) {
        for (int x = 0; x < a->width; ++x) {
            if (x == 0 || y == 0 || x == a->width - 1 || y == a->height - 1) {
                set_tile(a, x, y, TILE_CLAIMED);
            }
        }
    }
    a->claimed_interior = 0;
    a->drawing = false;
    a->trail_started = false;
    a->dead = false;
    a->tick = 0;
    a->ghost_enemy = -1;
    a->ghost_remaining = 0;
    a->ghost_cooldown = a->ghost_interval;
    a->ambient_cooldown = 95 + rand() % 55;
    a->scare_remaining = 0;
    a->scare_is_warning = false;
    a->player_x = a->width / 2;
    a->player_y = 0;
    a->move_x = 0;
    a->move_y = 0;

    for (int n = 0; n < a->enemy_count; ++n) {
        ArenaEnemy *e = &a->enemies[n];
        int retries = 0;
        do {
            e->x = 1 + rand() % (a->width - 2);
            e->y = 1 + rand() % (a->height - 2);
            ++retries;
        } while (enemy_occupies(a, n, e->x, e->y) && retries < 50);
        e->dx = rand() % 2 ? 1 : -1;
        e->dy = rand() % 2 ? 1 : -1;
    }
    return true;
}

void arena_set_direction(Arena *a, int dx, int dy) {
    if (a && !a->dead && dx >= -1 && dx <= 1 && dy >= -1 && dy <= 1 &&
        (dx == 0 || dy == 0)) {
        a->move_x = dx;
        a->move_y = dy;
    }
}

/* Toggle double-speed horizontal travel. The setting persists between levels. */
void arena_toggle_horizontal_boost(Arena *a) {
    if (a && !a->dead) a->horizontal_boost = !a->horizontal_boost;
}

void arena_start_drawing(Arena *a) {
    if (a && !a->dead && !a->drawing &&
        arena_tile(a, a->player_x, a->player_y) == TILE_CLAIMED) {
        a->drawing = true;
    }
}

/* Mark every EMPTY cell reachable from enemies. Everything else is enclosed
 * territory and can be claimed. We perform a single multi-source flood fill. */
static void complete_capture(Arena *a) {
    const int count = a->width * a->height;
    int head = 0, tail = 0;
    const int dx[] = {0, 0, -1, 1};
    const int dy[] = {-1, 1, 0, 0};
    memset(a->visited, 0, (size_t)count);
    for (int n = 0; n < a->enemy_count; ++n) {
        ArenaEnemy *e = &a->enemies[n];
        int index = index_of(a, e->x, e->y);
        if (a->tiles[index] == TILE_EMPTY && !a->visited[index]) {
            a->visited[index] = 1;
            a->queue[tail++] = index;
        }
    }
    while (head < tail) {
        int p = a->queue[head++];
        int x = p % a->width, y = p / a->width;
        for (int d = 0; d < 4; ++d) {
            int nx = x + dx[d], ny = y + dy[d];
            if (!inside(a, nx, ny)) continue;
            int ni = index_of(a, nx, ny);
            if (a->tiles[ni] != TILE_EMPTY || a->visited[ni]) continue;
            a->visited[ni] = 1;
            a->queue[tail++] = ni;
        }
    }
    for (int y = 1; y < a->height - 1; ++y) {
        for (int x = 1; x < a->width - 1; ++x) {
            int i = index_of(a, x, y);
            if (a->tiles[i] == TILE_TRAIL ||
                (a->tiles[i] == TILE_EMPTY && !a->visited[i])) {
                a->tiles[i] = TILE_CLAIMED;
                a->claimed_interior++;
                a->score += 100;
            }
        }
    }
}

/* When an intangible enemy materializes inside a wall, move it to the nearest
 * free cell, so the regular collision code never leaves it trapped. */
static void materialize_ghost(Arena *a) {
    if (a->ghost_enemy < 0) return;
    ArenaEnemy *e = &a->enemies[a->ghost_enemy];
    if (arena_tile(a, e->x, e->y) == TILE_EMPTY) return;
    int nearest = INT_MAX;
    int bestx = -1, besty = -1;
    for (int y = 1; y < a->height - 1; ++y) {
        for (int x = 1; x < a->width - 1; ++x) {
            if (arena_tile(a, x, y) != TILE_EMPTY) continue;
            int dist = abs(x - e->x) + abs(y - e->y);
            if (dist < nearest) {
                nearest = dist;
                bestx = x;
                besty = y;
            }
        }
    }
    if (bestx >= 0) {
        e->x = bestx;
        e->y = besty;
    }
}

static void ghost_timers(Arena *a) {
    if (a->scare_remaining > 0) --a->scare_remaining;
    if (a->ambient_cooldown > 0) --a->ambient_cooldown;
    if (a->ambient_cooldown == 0) {
        a->ambient_cooldown = 100 + rand() % 70; /* approximately 10-17s */
        if (a->ascii_enabled && a->ghost_remaining == 0) {
            a->scare_remaining = TICKS_PER_AMBIENT_SCARE;
            a->scare_is_warning = false;
        }
    }
    if (!a->ghosts_enabled) return;
    if (a->ghost_remaining > 0) {
        --a->ghost_remaining;
        if (a->ghost_remaining == 0) {
            materialize_ghost(a);
            a->ghost_enemy = -1;
            a->ghost_cooldown = a->ghost_interval;
        }
    } else {
        if (a->ghost_cooldown > 0) --a->ghost_cooldown;
        if (a->ghost_cooldown == 0 && a->enemy_count > 0) {
            a->ghost_enemy = rand() % a->enemy_count;
            a->ghost_remaining = a->ghost_duration;
            if (a->ascii_enabled) {
                a->scare_remaining = TICKS_PER_GHOST_WARNING;
                a->scare_is_warning = true;
            }
        }
    }
}

/* Walls block regular enemies. A ghost ignores claimed territory, but not
 * the outer edge. A trail still kills the player when touched by an enemy. */
static void move_enemy(Arena *a, int n) {
    ArenaEnemy *e = &a->enemies[n];
    bool ghost = (a->ghost_enemy == n);
    int nx = e->x + e->dx, ny = e->y + e->dy;
    if (nx <= 0 || nx >= a->width - 1 ||
        (!ghost && arena_tile(a, nx, e->y) == TILE_CLAIMED)) {
        e->dx = -e->dx;
        nx = e->x + e->dx;
    }
    if (ny <= 0 || ny >= a->height - 1 ||
        (!ghost && arena_tile(a, e->x, ny) == TILE_CLAIMED)) {
        e->dy = -e->dy;
        ny = e->y + e->dy;
    }
    /* A corner can still be blocked after bouncing. In that case, stay put. */
    if (nx <= 0 || nx >= a->width - 1 ||
        (!ghost && arena_tile(a, nx, e->y) == TILE_CLAIMED)) nx = e->x;
    if (ny <= 0 || ny >= a->height - 1 ||
        (!ghost && arena_tile(a, e->x, ny) == TILE_CLAIMED)) ny = e->y;
    if (!ghost && arena_tile(a, nx, ny) == TILE_CLAIMED) {
        e->dx = -e->dx;
        e->dy = -e->dy;
        return;
    }
    e->x = nx;
    e->y = ny;
    if (arena_tile(a, nx, ny) == TILE_TRAIL ||
        (a->player_x == nx && a->player_y == ny &&
         (ghost || arena_tile(a, nx, ny) != TILE_CLAIMED))) a->dead = true;
}

static void move_player(Arena *a) {
    if (a->move_x == 0 && a->move_y == 0) return;
    /* A boosted horizontal tick crosses two cells, but must check and claim
     * EACH tile separately. Never teleport past an enemy, trail, or wall. */
    const int steps = (a->horizontal_boost && a->move_x != 0) ? 2 : 1;
    for (int step = 0; step < steps; ++step) {
        int nx = a->player_x + a->move_x;
        int ny = a->player_y + a->move_y;
        if (!inside(a, nx, ny)) return;
        Tile destination = arena_tile(a, nx, ny);
        if (destination == TILE_EMPTY && !a->drawing) return;
        if (destination == TILE_TRAIL) {
            a->dead = true;
            return;
        }
        /* Check intermediate tiles, even at double speed. */
        for (int n = 0; n < a->enemy_count; ++n) {
            if (a->enemies[n].x == nx && a->enemies[n].y == ny &&
                (a->ghost_enemy == n || destination != TILE_CLAIMED)) {
                a->dead = true;
                return;
            }
        }
        a->player_x = nx;
        a->player_y = ny;
        if (a->drawing) {
            if (destination == TILE_EMPTY) {
                set_tile(a, nx, ny, TILE_TRAIL);
                a->trail_started = true;
            } else if (destination == TILE_CLAIMED && a->trail_started) {
                complete_capture(a);
                a->drawing = false;
                a->trail_started = false;
                return; /* Stop on the boundary that completed the claim. */
            }
        }
    }
}

void arena_tick(Arena *a) {
    if (!a || !a->tiles || a->dead || arena_level_complete(a)) return;
    ++a->tick;
    ghost_timers(a);
    move_player(a);
    if (a->dead) return;
    if (a->tick % 2 == 0) {
        for (int n = 0; n < a->enemy_count && !a->dead; ++n) {
            move_enemy(a, n);
        }
    }
    /* A ghost that phases into the stationary player also kills them. */
    if (!a->dead && a->ghost_enemy >= 0 &&
        a->enemies[a->ghost_enemy].x == a->player_x &&
        a->enemies[a->ghost_enemy].y == a->player_y) a->dead = true;
}

int arena_percent(const Arena *a) {
    return a && a->interior_total > 0
         ? (100 * a->claimed_interior) / a->interior_total : 0;
}

bool arena_level_complete(const Arena *a) {
    return a && a->interior_total > 0 &&
           a->claimed_interior * 2 > a->interior_total;
}
