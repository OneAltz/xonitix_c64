/* Simple platform-specific console input and timing, implemented in C. */
#if !defined(_WIN32)
#define _POSIX_C_SOURCE 200809L
#endif
#include "terminal.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>

static HANDLE output_handle;
static CONSOLE_CURSOR_INFO saved_cursor;
static bool saved_cursor_valid;
static bool started;
static DWORD previous_mode;
static bool previous_mode_valid;

bool terminal_start(void) {
    output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (output_handle == INVALID_HANDLE_VALUE ||
        !GetConsoleCursorInfo(output_handle, &saved_cursor)) {
        return false;
    }
    saved_cursor_valid = true;
    CONSOLE_CURSOR_INFO hidden = saved_cursor;
    hidden.bVisible = FALSE;
    if (!SetConsoleCursorInfo(output_handle, &hidden)) {
        return false;
    }
    if (GetConsoleMode(output_handle, &previous_mode)) {
        previous_mode_valid = true;
        if (!SetConsoleMode(output_handle,
                            previous_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
            SetConsoleCursorInfo(output_handle, &saved_cursor);
            return false;
        }
    } else {
        SetConsoleCursorInfo(output_handle, &saved_cursor);
        return false;
    }
    started = true;
    return true;
}

void terminal_stop(void) {
    if (started && saved_cursor_valid) {
        SetConsoleCursorInfo(output_handle, &saved_cursor);
    }
    if (started && previous_mode_valid) SetConsoleMode(output_handle, previous_mode);
    started = false;
}

int terminal_width(void) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        return info.srWindow.Right - info.srWindow.Left + 1;
    }
    return 80;
}

int terminal_height(void) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        return info.srWindow.Bottom - info.srWindow.Top + 1;
    }
    return 24;
}

GameKey terminal_poll_key(void) {
    if (!_kbhit()) {
        return KEY_NONE;
    }
    int k = _getch();
    if (k == 0 || k == 224) {
        if (!_kbhit()) {
            return KEY_NONE;
        }
        k = _getch();
        if (k == 72) return KEY_UP;
        if (k == 75) return KEY_LEFT;
        if (k == 77) return KEY_RIGHT;
        if (k == 80) return KEY_DOWN;
        return KEY_NONE;
    }
    if (k == ' ') return KEY_FIRE;
    if (k == 'x' || k == 'X') return KEY_STOP;
    if (k == 'b' || k == 'B') return KEY_BOOST;
    if (k == 'w' || k == 'W') return KEY_UP;
    if (k == 'a' || k == 'A') return KEY_LEFT;
    if (k == 's' || k == 'S') return KEY_DOWN;
    if (k == 'd' || k == 'D') return KEY_RIGHT;
    if (k == 'q' || k == 'Q') return KEY_QUIT;
    return KEY_NONE;
}

void terminal_sleep_ms(unsigned int milliseconds) {
    Sleep(milliseconds);
}

double terminal_seconds(void) {
    return GetTickCount64() / 1000.0;
}

void terminal_clear_line(void) {
    DWORD written;
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(output_handle, &info)) return;
    COORD beginning = {0, info.dwCursorPosition.Y};
    DWORD width = (DWORD)(info.srWindow.Right - info.srWindow.Left + 1);
    FillConsoleOutputCharacterA(output_handle, ' ', width, beginning, &written);
    SetConsoleCursorPosition(output_handle, beginning);
}

#else
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

static struct termios saved_mode;
static bool started;
static int escape_state;

bool terminal_start(void) {
    struct termios raw;
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO) ||
        tcgetattr(STDIN_FILENO, &saved_mode) != 0) {
        return false;
    }
    raw = saved_mode;
    raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
        return false;
    }
    started = true;
    printf("\033[?25l");
    fflush(stdout);
    return true;
}

void terminal_stop(void) {
    if (!started) return;
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_mode);
    printf("\033[?25h");
    fflush(stdout);
    started = false;
}

int terminal_width(void) {
    struct winsize size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0 && size.ws_col > 0) {
        return size.ws_col;
    }
    return 80;
}

int terminal_height(void) {
    struct winsize size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0 && size.ws_row > 0) {
        return size.ws_row;
    }
    return 24;
}

/* Consume all pending bytes. Escape sequences are handled across polls,
 * without waiting on a blocking read for the remaining arrow-key bytes. */
GameKey terminal_poll_key(void) {
    /* Return one complete key per poll. Do not discard Space when the player
     * presses Space+direction in quick succession. Escape sequences remain
     * stateful across reads, without introducing a blocking wait. */
    for (;;) {
        fd_set inputs;
        struct timeval timeout = {0, 0};
        unsigned char byte;
        FD_ZERO(&inputs);
        FD_SET(STDIN_FILENO, &inputs);
        if (select(STDIN_FILENO + 1, &inputs, NULL, NULL, &timeout) <= 0)
            return KEY_NONE;
        if (read(STDIN_FILENO, &byte, 1) != 1) return KEY_NONE;
        if (escape_state == 1) {
            escape_state = (byte == '[' || byte == 'O') ? 2 : 0;
            continue;
        }
        if (escape_state == 2) {
            escape_state = 0;
            if (byte == 'D') return KEY_LEFT;
            if (byte == 'C') return KEY_RIGHT;
            if (byte == 'B') return KEY_DOWN;
            if (byte == 'A') return KEY_UP;
            continue;
        }
        if (byte == 27) escape_state = 1;
        else if (byte == ' ') return KEY_FIRE;
        else if (byte == 'x' || byte == 'X') return KEY_STOP;
        else if (byte == 'b' || byte == 'B') return KEY_BOOST;
        else if (byte == 'w' || byte == 'W') return KEY_UP;
        else if (byte == 'a' || byte == 'A') return KEY_LEFT;
        else if (byte == 's' || byte == 'S') return KEY_DOWN;
        else if (byte == 'd' || byte == 'D') return KEY_RIGHT;
        else if (byte == 'q' || byte == 'Q') return KEY_QUIT;
    }
}

void terminal_sleep_ms(unsigned int milliseconds) {
    struct timespec duration;
    duration.tv_sec = (time_t)(milliseconds / 1000);
    duration.tv_nsec = (long)(milliseconds % 1000) * 1000000L;
    while (nanosleep(&duration, &duration) == -1 && errno == EINTR) {}
}

double terminal_seconds(void) {
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) return 0;
    return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
}

void terminal_clear_line(void) {
    printf("\r\033[2K");
}
#endif
