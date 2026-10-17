# Xonitix in C

A complete C11 rewrite of the single-file C++ terminal game [Xonitix](https://github.com/OneAltz/Xonitix56). Original game by Breno G. de Oliveira (2018), MIT License.

Xonitix is a one-line arcade game. Move left and right, press Space to claim territory without hitting a moving dot, and claim more than 50% of the line to advance. Each level adds one more enemy.

## Build on Linux, macOS, or BSD

```sh
make
./xonitix
```

Without `make`:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic main.c game.c terminal.c -o xonitix
./xonitix
```

## Build on Windows

With MinGW-w64:

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic main.c game.c terminal.c -o xonitix.exe
.\xonitix.exe
```

Use an actual interactive terminal. No third-party libraries are required.

## Controls

- Left arrow: move left until stopped or directed otherwise
- Right arrow: move right until stopped or directed otherwise
- Down arrow: stop moving
- Space: claim territory
- Q: quit

## Options

- `-l 80`: choose line length (2 to 4096)
- `-q`: suppress opening instruction line
- `-s`: erase game line at exit and suppress opening instruction line
- `-h`: help

Example:

```sh
./xonitix -l 60 -q
```

## Tests

Run `make test`. Tests cover claiming and scoring, leveling, fatal shots, enemy wall bounces, collisions on both sides, and boundaries.

## Porting notes

- Pure C11 structures and functions replace C++ classes, `std::vector`, `std::string`, references, and `<chrono>`.
- On POSIX, input uses `termios`, `select`, and `read`; on Windows, `_kbhit` and `_getch`.
- A monotonic wall clock replaces `std::clock`, avoiding machine-dependent CPU-time pacing.
- The gameplay keeps the original 10 ms POSIX and 30 ms Windows simulation update intervals. It redraws about every 5 ms when idle.
- Every cell can be claimed only once. Unlike the C++ implementation, repeated fills do not duplicate wall positions or award points for cells already filled. This prevents unintended score inflation and level progression.
- An enemy within one cell of a shot is fatal regardless of enemy-array ordering.
- The advertised `-q` option is implemented (the original README documents it, though the C++ implementation does not handle it).
- The terminal is restored on normal exit and interrupt signals.
- This project was tested using GCC on Linux. The Windows branch is provided but was not runtime-tested on Windows.

## License

Original game Copyright (c) 2018 Breno G. de Oliveira. MIT. See `LICENSE` for the required attribution and license text.
