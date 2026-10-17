#include "../game.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void setup(Game *game) {
    srand(123);
    assert(game_init(game, 60));
    assert(game_next_level(game));
    assert(game->level == 1 && game->enemy_count == 1);
}

static void test_claim_and_level(void) {
    Game game;
    setup(&game);
    game.hero_pos = 30;
    game.enemies[0].pos = 20;
    game.enemies[0].direction = -1;
    game.enemies[0].speed = 1;
    game_fire(&game);
    game_tick(&game);
    assert(!game.dead);
    assert(game.claimed == 31);
    assert(game.score == 3100);
    assert(game_level_complete(&game));
    assert(game.walls[30] && game.walls[60] && !game.walls[29]);
    game_fire(&game);
    game_tick(&game);
    assert(game.claimed == 31 && game.score == 3100);
    assert(game_next_level(&game));
    assert(game.level == 2 && game.enemy_count == 2);
    assert(game.claimed == 0 && game.score == 3100);
    game_free(&game);
}

static void test_close_shot_is_fatal(void) {
    Game game;
    setup(&game);
    game.hero_pos = 30;
    game.enemies[0].pos = 29;
    game_fire(&game);
    game_tick(&game);
    assert(game.dead && game.score == 0);
    game_free(&game);
}

static void test_bounce(void) {
    Game game;
    setup(&game);
    game.walls[10] = 1;
    game.claimed = 1;
    game.enemies[0].pos = 8;
    game.enemies[0].speed = 2;
    game.enemies[0].direction = 1;
    game_tick(&game);
    assert(game.enemies[0].pos == 9);
    assert(game.enemies[0].direction == -1);
    game_free(&game);
}

static void test_both_sides_blocked(void) {
    Game game;
    setup(&game);
    assert(game_next_level(&game));
    game.hero_pos = 30;
    game.enemies[0].pos = 20;
    game.enemies[1].pos = 40;
    game_fire(&game);
    game_tick(&game);
    assert(!game.dead);
    assert(game.claimed == 1 && game.walls[30]);
    assert(!game_level_complete(&game));
    game_free(&game);
}

static void test_boundaries_and_invalid_dimensions(void) {
    Game game;
    assert(!game_init(&game, 1));
    setup(&game);
    game.hero_pos = game.width;
    game_move(&game, 1);
    game_tick(&game);
    assert(game.hero_pos == game.width && game.hero_direction == 0);
    game.hero_pos = 0;
    game_move(&game, -1);
    game_tick(&game);
    assert(game.hero_pos == 0 && game.hero_direction == 0);
    game_free(&game);
}

int main(void) {
    test_claim_and_level();
    test_close_shot_is_fatal();
    test_bounce();
    test_both_sides_blocked();
    test_boundaries_and_invalid_dimensions();
    puts("All Xonitix C game-engine tests passed.");
    return EXIT_SUCCESS;
}
