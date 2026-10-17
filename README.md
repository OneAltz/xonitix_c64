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




## Windows 10/11 Installation

Xonitix Trick or Treat Edition supports Windows using the C11 programming language. You will need a C compiler to build the game.

### Step 1: Install GCC

1. Download and install [MSYS2](https://www.msys2.org/).
2. Open **MSYS2 UCRT64** from the Windows Start menu.
3. Install GCC using:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc
```

Verify the installation:

```bash
gcc --version
```

### Step 2: Download and compile the game

Download and extract the repository, then navigate to the folder containing the source files.

Compile the Halloween edition:

```bash
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic main.c arena.c terminal.c -o xonitix.exe
```

This creates `xonitix.exe`.

### Step 3: Launch the game

Open Windows Terminal or PowerShell in the project directory and enter:

```powershell
.\xonitix.exe
```

To test the Halloween Ghost Mode:

```powershell
.\xonitix.exe --ghost-interval 3 --ghost-duration 5
```

### Step 4: Game controls

| Key | Action |
|---|---|
| Arrow keys or WASD | Move |
| Space | Begin capturing territory |
| X | Stop moving |
| Q | Quit |

### Troubleshooting

- **GCC not found:** Make sure GCC is installed and you are compiling from MSYS2 UCRT64.
- **Missing source files:** Verify you are in the directory containing `main.c`, `arena.c`, and `terminal.c`.
- **Terminal too small:** Try `.\xonitix.exe -l 40 -r 12`.
- **Game fails to start:** Use Windows Terminal or PowerShell instead of the MSYS2 terminal window.

**Note:** Windows support has been implemented but has not yet been tested on a Windows machine. Windows testing and bug reports are welcome.


