#ifndef XONITIX_ARENA_H
#define XONITIX_ARENA_H

#include <stdbool.h>
#include <stddef.h>

#define ARENA_MIN_WIDTH 12
#define ARENA_MAX_WIDTH 120
#define ARENA_MIN_HEIGHT 8
#define ARENA_MAX_HEIGHT 45
#define ARENA_MAX_ENEMIES 12

/* ASCII game grid. The perimeter is safe, permanently claimed territory. */
typedef enum {
    TILE_EMPTY = 0,
    TILE_CLAIMED = 1,
    TILE_TRAIL = 2
} Tile;

typedef struct {
    int x, y;
    int dx, dy;
} ArenaEnemy;

/* The art is cosmetic. GHOST is reserved for real Ghost Mode warnings. */
typedef enum {
    ART_GHOST = 0,
    ART_SPIDER,
    ART_BAT,
    ART_PUMPKIN,
    ART_SKULL,
    ART_CAT,
    ART_COUNT
} HalloweenArt;

typedef struct {
    int width, height;
    int level;
    long long score;
    unsigned char *tiles;
    unsigned char *visited;
    int *queue;
    int claimed_interior;
    int interior_total;
    int player_x, player_y;
    int move_x, move_y;
    bool horizontal_boost;      /* B toggles 2x left/right speed. */
    bool drawing;
    bool trail_started;
    bool dead;
    ArenaEnemy enemies[ARENA_MAX_ENEMIES];
    int enemy_count;
    unsigned int tick;
    int ghost_enemy;           /* -1 when inactive */
    int ghost_remaining;       /* Timers use 100ms ticks */
    int ghost_cooldown;
    int ghost_interval;        /* Ticks between ghost events */
    int ghost_duration;
    int scare_remaining;       /* Nonblocking ASCII display */
    int ambient_cooldown;
    int ambient_interval;    /* 0 = random 10 to 17 seconds; otherwise ticks */
    HalloweenArt scare_art; /* What to render during this nonblocking popup */
    bool ascii_enabled;
    bool ghosts_enabled;
    bool scare_is_warning;
} Arena;

bool arena_init(Arena *arena, int width, int height);
void arena_free(Arena *arena);
bool arena_next_level(Arena *arena);
void arena_set_direction(Arena *arena, int dx, int dy);
void arena_toggle_horizontal_boost(Arena *arena);
void arena_start_drawing(Arena *arena);
void arena_tick(Arena *arena);
bool arena_level_complete(const Arena *arena);
Tile arena_tile(const Arena *arena, int x, int y);
int arena_percent(const Arena *arena);

#endif
