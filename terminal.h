#ifndef XONITIX_TERMINAL_H
#define XONITIX_TERMINAL_H

#include <stdbool.h>

typedef enum {
    KEY_NONE,
    KEY_LEFT,
    KEY_UP,
    KEY_RIGHT,
    KEY_DOWN,
    KEY_FIRE,
    KEY_STOP,
    KEY_BOOST,
    KEY_QUIT
} GameKey;

bool terminal_start(void);
void terminal_stop(void);
int terminal_width(void);
int terminal_height(void);
GameKey terminal_poll_key(void);
void terminal_sleep_ms(unsigned int milliseconds);
double terminal_seconds(void);
void terminal_clear_line(void);

#endif
