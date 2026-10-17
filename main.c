/* Xonitix 2D Halloween edition in ISO C11.
 * Copyright 2018 Breno G. de Oliveira (original), MIT License.
 * See LICENSE. */
#include "arena.h"
#include "terminal.h"

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define VERSION "2.0-halloween-c"
#define SIMULATION_STEP 0.10

static volatile sig_atomic_t interrupted;
static void on_signal(int signal_number) {
    (void)signal_number;
    interrupted = 1;
}

static bool integer_option(const char *value, int min, int max, int *dest) {
    char *end = NULL;
    long result;
    if (!value || !*value) return false;
    errno = 0;
    result = strtol(value, &end, 10);
    if (errno || value == end || *end || result < min || result > max || result > INT_MAX)
        return false;
    *dest = (int)result;
    return true;
}

static void usage(const char *program) {
    printf("%s %s\n", program, VERSION);
    puts("2D Trick or Treat Xonitix (C11; no third-party libraries).\n"
         "Capture territory by tracing a path from claimed ground and returning.\n"
         "Ghosts temporarily ignore walls. Ghost ASCII appears as a warning\n"
         "and occasionally as harmless spiders, bats, pumpkins, skulls and cats.\n\n"
         "Move: Arrow keys or WASD    Stop: X    B: horizontal 2x boost\n"
         "Start a capture: Space (while on #)    Quit: Q\n"
         "Claim more than 50% of the interior to level up.\n\n"
         "Options:\n"
         "  -l N                  Board width (12..120)\n"
         "  -r N                  Board height (8..45)\n"
         "  --ghost-interval N    Seconds between ghost events (1..300, default 20)\n"
         "  --ghost-duration N    Ghost duration in seconds (1..300, default 5)\n"
         "  --ascii-interval N    Show random Halloween art every N seconds (1..300)\n"
         "  --no-ghost            Disable ghost events\n"
         "  --no-ascii            Disable ASCII jumpscares and warnings\n"
         "  -s                    Clear board on exit (stealth)\n"
         "  -h, --help            Print help\n\n"
         "The original one-line C game remains available as xonitix-classic.");
}

/* The art never blocks play. It stays in a side panel, not over the board. */
typedef struct {
    const char *lines[6];
    const char *caption;
} ArtPanel;

/* Six-line designs fit beside even the minimum-height board. All characters
 * are ASCII and every row is small enough for the existing 24-column panel. */
static const ArtPanel halloween_art[ART_COUNT] = {
    [ART_GHOST] = {{
        "      .-.       ",
        "     (o o)      ",
        "     | O |      ",
        "     |   |      ",
        "     '~~~'      ",
        "      B O O     "
    }, "GHOST UNLEASHED!"},
    [ART_SPIDER] = {{
        "  \\   /\\   /  ",
        "   \\ /  \\ /   ",
        "   (  o  o  )  ",
        "   /  /\\  \\   ",
        "  /__/ || \\__ ",
        "      /  \\      "
    }, "SPIDER SENSE!"},
    [ART_BAT] = {{
        "    /\\    /\\    ",
        "   /  \\__/  \\   ",
        "  /  (o  o)  \\  ",
        "  \\    \\/    /  ",
        "   \\  /  \\  /   ",
        "    \\/    \\/    "
    }, "BAT INCOMING!"},
    [ART_PUMPKIN] = {{
        "      __        ",
        "   .-'  '-.     ",
        "  /  ^  ^  \\    ",
        " |   \\__/   |   ",
        "  \\  ____  /    ",
        "   '------'     "
    }, "PUMPKIN PARTY!"},
    [ART_SKULL] = {{
        "    .------.    ",
        "   /  x  x  \\   ",
        "  |    __    |  ",
        "  |   (__)   |  ",
        "   \\  ||||  /   ",
        "    '------'    "
    }, "SPOOKY SKULL!"},
    [ART_CAT] = {{
        "    /\\_/\\       ",
        "   ( o.o )      ",
        "    > ^ <       ",
        "   /  _  \\      ",
        "  (__/ \\__)     ",
        "    M E O W     "
    }, "BLACK CAT!"}
};

static char tile_symbol(const Arena *arena, int x, int y) {
    Tile tile = arena_tile(arena, x, y);
    if (tile == TILE_CLAIMED) return '#';
    if (tile == TILE_TRAIL) return '+';
    return ' ';
}

static void render(const Arena *a, int columns) {
    const bool side_panel = columns >= a->width + 24;
    const bool warning = a->ghost_remaining > 0;
    printf("\033[H\033[2KXONITIX 2D | Level %d | Score %lld | Filled %d%%\n",
           a->level, a->score, arena_percent(a));
    printf("\033[2KGhost: ");
    if (!a->ghosts_enabled) {
        printf("OFF");
    } else if (warning) {
        printf("ACTIVE %d.%ds", a->ghost_remaining / 10,
               a->ghost_remaining % 10);
    } else {
        printf("in %d.%ds", a->ghost_cooldown / 10,
               a->ghost_cooldown % 10);
    }
    printf(" | Horizontal boost: %s (B)", a->horizontal_boost ? "ON" : "OFF");
    if (columns < 90) printf("\n");
    else printf(" | Space draw | X stop | Q quit\n");

    for (int y = 0; y < a->height; ++y) {
        printf("\033[2K");
        for (int x = 0; x < a->width; ++x) {
            char symbol = tile_symbol(a, x, y);
            for (int i = 0; i < a->enemy_count; ++i) {
                if (a->enemies[i].x == x && a->enemies[i].y == y) {
                    symbol = (a->ghost_enemy == i) ? 'G' : '.';
                }
            }
            if (a->player_x == x && a->player_y == y) symbol = '@';
            putchar(symbol);
        }
        if (side_panel) {
            if (a->ascii_enabled && a->scare_remaining > 0) {
                HalloweenArt art = a->scare_art;
                if (art < ART_GHOST || art >= ART_COUNT) art = ART_GHOST;
                if (y < 6) printf("   %s", halloween_art[art].lines[y]);
                else if (y == 6) printf("   %s", halloween_art[art].caption);
            }
        }
        putchar('\n');
    }
    printf("\033[2K%s\n", a->drawing
          ? "DRAWING: return to # to capture. Avoid enemy dots and ghosts!"
          : "@ player   # territory   + active trail   . enemy   G ghost");
    fflush(stdout);
}

int main(int argc, char **argv) {
    int width = terminal_width() - 27;
    int height = terminal_height() - 5;
    int ghost_interval = 20, ghost_duration = 5, ascii_interval = 0;
    bool stealth = false, no_ascii = false, no_ghost = false;
    int result = EXIT_SUCCESS;
    Arena arena;

    if (width > 62) width = 62;
    if (height > 20) height = 20;
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if (!strcmp(arg, "--help") || !strcmp(arg, "-h")) {
            usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (!strcmp(arg, "-l")) {
            if (++i == argc || !integer_option(argv[i], ARENA_MIN_WIDTH,
                                                 ARENA_MAX_WIDTH, &width)) {
                fputs("Invalid -l: expected width between 12 and 120.\n", stderr);
                return EXIT_FAILURE;
            }
        } else if (!strcmp(arg, "-r")) {
            if (++i == argc || !integer_option(argv[i], ARENA_MIN_HEIGHT,
                                                 ARENA_MAX_HEIGHT, &height)) {
                fputs("Invalid -r: expected height between 8 and 45.\n", stderr);
                return EXIT_FAILURE;
            }
        } else if (!strcmp(arg, "--ghost-interval")) {
            if (++i == argc || !integer_option(argv[i], 1, 300, &ghost_interval)) {
                fputs("Invalid --ghost-interval: use seconds 1..300.\n", stderr);
                return EXIT_FAILURE;
            }
        } else if (!strcmp(arg, "--ghost-duration")) {
            if (++i == argc || !integer_option(argv[i], 1, 300, &ghost_duration)) {
                fputs("Invalid --ghost-duration: use seconds 1..300.\n", stderr);
                return EXIT_FAILURE;
            }
        } else if (!strcmp(arg, "--ascii-interval")) {
            if (++i == argc || !integer_option(argv[i], 1, 300, &ascii_interval)) {
                fputs("Invalid --ascii-interval: use seconds 1..300.\n", stderr);
                return EXIT_FAILURE;
            }
        } else if (!strcmp(arg, "--no-ascii")) no_ascii = true;
        else if (!strcmp(arg, "--no-ghost")) no_ghost = true;
        else if (!strcmp(arg, "-s")) stealth = true;
        else {
            fprintf(stderr, "Unknown option %s. Try -h.\n", arg);
            return EXIT_FAILURE;
        }
    }

    if (width < ARENA_MIN_WIDTH || height < ARENA_MIN_HEIGHT ||
        width > ARENA_MAX_WIDTH || height > ARENA_MAX_HEIGHT) {
        fputs("Terminal is too small. Resize it or use -l 40 -r 12.\n", stderr);
        return EXIT_FAILURE;
    }
    if (width > terminal_width() - (no_ascii ? 0 : 24) ||
        height + 3 > terminal_height()) {
        fputs("Board or ghost art will not fit. Resize terminal or use smaller -l/-r.\n", stderr);
        return EXIT_FAILURE;
    }
    srand((unsigned)time(NULL));
    if (!arena_init(&arena, width, height)) {
        fputs("Unable to allocate the 2D board.\n", stderr);
        return EXIT_FAILURE;
    }
    arena.ambient_interval = ascii_interval * 10;
    arena.ghost_interval = ghost_interval * 10;
    arena.ghost_duration = ghost_duration * 10;
    arena.ghosts_enabled = !no_ghost;
    arena.ascii_enabled = !no_ascii;
    if (!arena_next_level(&arena) || !terminal_start()) {
        fputs("Unable to start game. Use an interactive terminal.\n", stderr);
        arena_free(&arena);
        return EXIT_FAILURE;
    }
    atexit(terminal_stop);
    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    printf("\033[2J\033[H");

    double previous = terminal_seconds(), accumulator = 0;
    while (!interrupted && !arena.dead) {
        GameKey key = terminal_poll_key();
        switch (key) {
            case KEY_QUIT: arena.dead = true; break;
            case KEY_LEFT: arena_set_direction(&arena, -1, 0); break;
            case KEY_RIGHT: arena_set_direction(&arena, 1, 0); break;
            case KEY_UP: arena_set_direction(&arena, 0, -1); break;
            case KEY_DOWN: arena_set_direction(&arena, 0, 1); break;
            case KEY_STOP: arena_set_direction(&arena, 0, 0); break;
            case KEY_BOOST: arena_toggle_horizontal_boost(&arena); break;
            case KEY_FIRE: arena_start_drawing(&arena); break;
            case KEY_NONE: break;
        }
        double now = terminal_seconds();
        double elapsed = now - previous;
        previous = now;
        if (elapsed < 0) elapsed = 0;
        if (elapsed > 0.25) elapsed = 0.25;
        accumulator += elapsed;
        while (accumulator >= SIMULATION_STEP && !arena.dead &&
               !arena_level_complete(&arena)) {
            arena_tick(&arena);
            accumulator -= SIMULATION_STEP;
        }
        render(&arena, terminal_width());
        if (arena_level_complete(&arena) && !arena.dead) {
            printf("\033[2KLEVEL COMPLETE! Advancing...\n");
            fflush(stdout);
            terminal_sleep_ms(800);
            if (!arena_next_level(&arena)) {
                result = EXIT_FAILURE;
                break;
            }
            previous = terminal_seconds();
            accumulator = 0;
            printf("\033[2J\033[H");
        } else {
            terminal_sleep_ms(25);
        }
    }
    if (stealth) printf("\033[2J\033[H");
    else printf("\033[2K\nGAME OVER | Level %d | Score %lld\n",
                arena.level, arena.score);
    fflush(stdout);
    arena_free(&arena);
    return result;
}
