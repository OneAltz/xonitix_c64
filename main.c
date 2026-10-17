/* Xonitix 1.1, fully ported to C11.
 * Original game copyright 2018 Breno G. de Oliveira, MIT License. */
#include "game.h"
#include "terminal.h"

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define VERSION "1.1-c"
#ifdef _WIN32
#define UPDATE_SECONDS 0.030
#else
#define UPDATE_SECONDS 0.010
#endif

static volatile sig_atomic_t interrupted;

static void on_signal(int signum) {
    (void)signum;
    interrupted = 1;
}

static void show_usage(const char *program) {
    printf("%s %s\n\n", program, VERSION);
    puts("Xonitix is a one-line game where you gain points by claiming"
         " space while avoiding moving dots.\n"
         "Left/right arrows: move     Down arrow: stop\n"
         "Space: claim territory      Q: quit\n"
         "Fill more than 50% of the line to advance a level.\n\n"
         "Options:\n"
         "  -l N  Set line length (2 through 4096)\n"
         "  -q    Quiet mode (do not show opening instructions)\n"
         "  -s    Stealth mode (erase the game line when quitting)\n"
         "  -h    Show this help");
}

static bool parse_width(const char *value, int *width) {
    char *end;
    long parsed;
    errno = 0;
    parsed = strtol(value, &end, 10);
    if (errno != 0 || value == end || *end != '\0' ||
        parsed < 2 || parsed > 4096 || parsed > INT_MAX) return false;
    *width = (int)parsed;
    return true;
}

static void render_game(const Game *game) {
    char line[4098];
    const int width = game->width;
    memset(line, ' ', (size_t)width + 1);
    line[width + 1] = '\0';
    for (size_t i = 0; i < game->enemy_count; ++i) {
        int x = game->enemies[i].pos;
        if (x >= 0 && x <= width) line[x] = line[x] == ' ' ? '.' : ':';
    }
    for (int i = 0; i <= width; ++i) {
        if (game->walls[i]) line[i] = '|';
    }
    if (game->hero_pos >= 0 && game->hero_pos <= width) {
        line[game->hero_pos] = (line[game->hero_pos] == '|') ? 'L' : '_';
    }
    terminal_clear_line();
    printf("%s| Level %d Score %lld", line, game->level, game->score);
    fflush(stdout);
}

int main(int argc, char **argv) {
    bool stealth = false;
    bool quiet = false;
    int width = terminal_width() - 30;
    int result = EXIT_SUCCESS;
    Game game;
    bool running = true;
    double last_time;
    double lag = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            show_usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "-s") == 0) {
            stealth = true;
        } else if (strcmp(argv[i], "-q") == 0) {
            quiet = true;
        } else if (strcmp(argv[i], "-l") == 0) {
            if (++i >= argc || !parse_width(argv[i], &width)) {
                fprintf(stderr, "Invalid or missing line length after -l (2..4096).\n");
                return EXIT_FAILURE;
            }
        } else {
            fprintf(stderr, "Unknown option: %s (try -h).\n", argv[i]);
            return EXIT_FAILURE;
        }
    }
    if (width < 2 || width > 4096) {
        fprintf(stderr, "Terminal is too narrow or too wide. Use -l N to set a line length.\n");
        return EXIT_FAILURE;
    }
    if (!game_init(&game, width)) {
        fprintf(stderr, "Could not allocate game state.\n");
        return EXIT_FAILURE;
    }
    srand((unsigned int)time(NULL));
    if (!game_next_level(&game)) {
        fprintf(stderr, "Could not create first level.\n");
        game_free(&game);
        return EXIT_FAILURE;
    }
    if (!terminal_start()) {
        fprintf(stderr, "Please run Xonitix in an interactive terminal.\n");
        game_free(&game);
        return EXIT_FAILURE;
    }
    atexit(terminal_stop);
    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    if (!quiet && !stealth) {
        puts("xonitix " VERSION ": arrows move, space claims, q quits. -h for usage");
    }
    last_time = terminal_seconds();
    while (running && !interrupted) {
        GameKey key = terminal_poll_key();
        if (key == KEY_QUIT) {
            game.dead = true;
            break;
        }
        if (key == KEY_LEFT) game_move(&game, -1);
        if (key == KEY_RIGHT) game_move(&game, 1);
        if (key == KEY_DOWN) game_move(&game, 0);
        if (key == KEY_FIRE) game_fire(&game);

        double now = terminal_seconds();
        double elapsed = now - last_time;
        last_time = now;
        if (elapsed < 0) elapsed = 0;
        if (elapsed > 0.25) elapsed = 0.25; /* Avoid huge catch-up bursts. */
        lag += elapsed;
        while (lag >= UPDATE_SECONDS && !game.dead && !game_level_complete(&game)) {
            game_tick(&game);
            lag -= UPDATE_SECONDS;
        }
        render_game(&game);
        if (game.dead) break;
        if (game_level_complete(&game)) {
            terminal_sleep_ms(2000);
            if (interrupted || !game_next_level(&game)) {
                if (!interrupted) {
                    fprintf(stderr, "\nCould not create next level.\n");
                    result = EXIT_FAILURE;
                }
                running = false;
            }
            last_time = terminal_seconds();
            lag = 0;
        } else {
            terminal_sleep_ms(5);
        }
    }
    if (stealth) {
        terminal_clear_line();
        putchar('\n');
    } else {
        puts("\n[G A M E  O V E R]");
        if (!interrupted) terminal_sleep_ms(2000);
    }
    game_free(&game);
    return result;
}
