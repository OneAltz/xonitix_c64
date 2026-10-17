# Xonitix: Trick or Treat Edition (Pure C11)

A Halloween upgrade to [Xonitix56](https://github.com/OneAltz/Xonitix56) with a new 2D arena and periodic ghosts. The original terminal game was made by Breno G. de Oliveira in 2018. Original source is MIT licensed; attribution and license are preserved in `LICENSE`.

## Trick: Ghost Mode + rotating Halloween ASCII art

- Approximately **20 seconds** after a level starts, one enemy becomes a ghost (`G`) for **5 seconds**.
- A ghost can pass through captured interior walls and harm the player even in safe territory. It cannot leave the arena's outer boundary.
- When a ghost activates, an ASCII ghost pops up on the right side of the board for **2 seconds** with a `GHOST UNLEASHED!` warning. **The game never pauses** for the display.
- Five harmless Halloween characters appear randomly between attacks: a **spider**, **bat**, **jack-o-lantern**, **skull**, or **black cat**. Each has its own ASCII drawing and caption.
- The extra artwork appears for **1.2 seconds**. By default it returns at randomized 10 to 17 second intervals, with the first appearing after approximately 9.5 to 14.9 seconds. You can override the interval with `--ascii-interval N` (whole seconds).
- The **original ghost ASCII art** is kept exclusively for actual ghost attacks. These other characters are visual only, not extra enemies. The art never blocks gameplay or hides the arena.
- The timer resets each level. Ghosts return to ordinary enemies at the end of their attack, moving to the nearest open cell if they happen to be inside claimed territory.
- `--no-ghost` disables ghost mechanics without disabling ambient Halloween art; `--no-ascii` disables all the artwork but keeps ghost gameplay enabled.

## Treat: Multi-line territory capture

This version is no longer restricted to a single line. The new default `xonitix` is a **2D board**.

- Move in all four directions.
- While standing on claimed territory (`#`), press **Space** to arm drawing.
- Move through unclaimed territory, creating a visible trail (`+`). Rejoin claimed territory to complete the capture.
- A flood-fill algorithm checks which empty regions contain enemies. Every enclosed, enemy-free region becomes claimed territory. Every newly claimed interior cell earns 100 points.
- Crossing your own unfinished trail or allowing an enemy to touch it ends the game. Normal enemies cannot pass claimed walls. Ghosts ignore those walls during the timed event.
- Claim **more than 50%** of interior tiles to advance to the next level. New levels add enemies, up to 12 simultaneous enemies.

**Legend**

```text
####################################
#       .                          #
#                                  #
#     ##########                   #
#     #        #          G        #
#     #        #                   #
#     ####@#####                   #
#                                  #
####################################

@ player  # claimed wall  + active trail  . enemy  G ghost
```

## New feature: Horizontal speed boost

Press **B** to toggle a **2x speed boost for left/right movement** (Left/Right arrows or A/D). Press **B** again to return to normal speed. Up/down movement remains unchanged. The boost begins OFF and stays in the chosen state between levels.

- Regular horizontal movement: **1 tile per simulation tick**.
- Boosted horizontal movement: **2 tiles per simulation tick**.
- Every crossed tile is still checked for enemies, trails, boundaries, and capture, so boosting cannot jump over hazards or miss claimed cells.
- The status line displays `Horizontal boost: ON (B)` or `Horizontal boost: OFF (B)`.

## Compiling (Fedora, Ubuntu, macOS, BSD)

```sh
make
make test
./xonitix
```

Without Make:

```sh
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic main.c arena.c terminal.c -o xonitix
./xonitix
```

To install GCC and Make if necessary on Fedora:

```sh
sudo dnf install gcc make
```

## Windows 10/11 (C11)

The recommended Windows compiler is **GCC using MSYS2 UCRT64**. You do not need C++ or a separate game library. The game uses the built-in Windows console API, so play it in **Windows Terminal** or **PowerShell**.

### 1. Install the compiler

1. Download and install [MSYS2](https://www.msys2.org/).
2. Open **MSYS2 UCRT64** from the Start menu, not the plain MSYS2 shell.
3. Update MSYS2, following any instructions to close and reopen the shell:

```sh
pacman -Syu
```

4. Install GCC:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc
gcc --version
```

### 2. Compile Xonitix

Download and extract the source ZIP or clone the GitHub repository. In the **MSYS2 UCRT64** shell, change to the folder containing `main.c`, `arena.c`, and `terminal.c`. For example, if the folder was extracted into Downloads:

```sh
cd /c/Users/YOUR_USERNAME/Downloads/xonitix-trick-or-treat
```

Replace `YOUR_USERNAME` with your Windows account folder name. If you cloned the project or extracted it elsewhere, use that path instead.

Compile the 2D Halloween game:

```sh
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic main.c arena.c terminal.c -o xonitix.exe
```

Optional: compile the preserved original one-line C game:

```sh
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic classic.c game.c terminal.c -o xonitix-classic.exe
```

### 3. Play in Windows Terminal / PowerShell

Open **Windows Terminal** with a **PowerShell** tab, then change to your extracted project folder:

```powershell
cd "$HOME\Downloads\xonitix-trick-or-treat"
.\xonitix.exe
```

For a quick demonstration of Ghost Mode and the ASCII warning:

```powershell
.\xonitix.exe --ghost-interval 3 --ghost-duration 5
```

If your window is too small, enlarge it or run:

```powershell
.\xonitix.exe -l 40 -r 12
```

See [Controls](#controls) for keyboard instructions and [Commands](#commands) for all options. **The `.exe` file is for Windows.** Linux and macOS users should build the non-`.exe` version with `make`.

### 4. Optional: run automated tests

In **MSYS2 UCRT64**, from the same source folder:

```sh
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic tests/test_arena.c arena.c -o test-arena.exe
./test-arena.exe

gcc -std=c11 -O2 -Wall -Wextra -Wpedantic tests/test_game.c game.c -o test-game.exe
./test-game.exe
```

### Windows troubleshooting

- **`gcc: command not found`**: Open **MSYS2 UCRT64**, not a regular PowerShell prompt, for the installation and compilation steps.
- **`main.c: No such file or directory`**: You are in the wrong folder. Change into the directory containing `main.c`, `arena.c`, and `terminal.c`.
- **`Unable to start game. Use an interactive terminal.`**: Launch the compiled executable in **Windows Terminal / PowerShell** instead of the MSYS2 MinTTY window, a redirected shell, or an editor's output pane.
- **Board or Halloween art will not fit**: Maximize the terminal, reduce its font size, or try `-l 40 -r 12`. The ASCII ghost occupies extra columns to the right of the arena.
- **PowerShell cannot find `xonitix.exe`**: Confirm that the executable was compiled in this folder and run it as `.\xonitix.exe`.

**Windows verification status:** Windows-specific input and console handling are implemented in `terminal.c`, but this edition has not been runtime-tested on Windows. Linux compilation and game-engine tests have passed. Windows testing and bug reports are welcome.

## Controls

| Key | Action |
| --- | --- |
| Up, Down, Left, Right arrows | Move |
| W, A, S, D | Also move |
| Space | Arm drawing while on captured territory |
| B | Toggle 2x left/right speed boost (does not affect up/down) |
| X | Stop moving |
| Q | Quit |

Direction continues until changed or stopped. You are safe while travelling inside claimed territory, except when Ghost Mode is active.

## Commands

```sh
./xonitix                              # Default 2D Trick or Treat game
./xonitix -l 40 -r 14                 # Custom board width and height
./xonitix --ghost-interval 3          # Demo: first ghost after 3 seconds
./xonitix --ghost-duration 10         # Ghost stays active for 10 seconds
./xonitix --ascii-interval 3           # Random Halloween art every 3 seconds
./xonitix --no-ascii                  # No ASCII artwork
./xonitix --no-ghost                  # No ghost gameplay mechanic
./xonitix -s                          # Clear board when exiting
./xonitix -h                          # Help
./xonitix-classic                     # Original single-line C port
```

The art appears to the right of the arena so it never obscures the game. A quick Halloween-art demo with gameplay Ghost Mode disabled is:

```sh
./xonitix -l 40 -r 12 --no-ghost --ascii-interval 2
```

The warning ghost still appears during actual Ghost Mode attacks when ghosts are enabled. If you want both visual surprises and ghosts during your demo:

```sh
./xonitix -l 40 -r 12 --ghost-interval 4 --ghost-duration 3 --ascii-interval 2
```

 Give your terminal enough width: at least **24 extra columns** beyond your requested board width. The arena needs 3 rows beyond its height for its status and legend.

## Complete command-line reference

| Argument | Meaning | Allowed values |
| --- | --- | --- |
| `-l N` | Width of the 2D board | 12 through 120 columns |
| `-r N` | Height of the 2D board | 8 through 45 rows |
| `--ghost-interval N` | Time between ghost attacks | 1 through 300 seconds; default 20 |
| `--ghost-duration N` | How long ghost attacks last | 1 through 300 seconds; default 5 |
| `--ascii-interval N` | Fixed time between random Halloween artwork | 1 through 300 seconds; defaults to random 10 through 17 |
| `--no-ghost` | Turn off ghost attacks | Flag, no value |
| `--no-ascii` | Hide decorative art and ghost warnings | Flag, no value |
| `-s` | Clear the terminal on exit | Flag, no value |
| `-h`, `--help` | Display game help | Flag, no value |

**The classic executable has different options.** Use `./xonitix-classic -h`. Classic supports `-l N` (line length 2 through 4096), `-q` (quiet start), `-s` (stealth exit), and `-h` / `--help`. Classic controls: Left and Right to move, Down to stop, Space to claim, Q to quit.

## Files

- `main.c`: 2D game runner, options, ASCII ghost artwork, and rendering.
- `arena.c` and `arena.h`: 2D movement, territory filling, enemies, ghost timers, collisions.
- `terminal.c` and `terminal.h`: non-blocking keyboard input, timing, terminal support.
- `classic.c`, `game.c`, `game.h`: original one-line C port, built as `xonitix-classic`.
- `tests/test_arena.c`, `tests/test_game.c`: automated 2D and classic game tests.
- `Makefile`: builds both games and runs both test suites.
- `LICENSE`: original MIT license and attribution.

## Verification

Both games compile with `-std=c11 -Wall -Wextra -Wpedantic -Werror`. The 2D engine's tests cover horizontal boost including intermediate collisions and captures, flood fill, territory capture, wall phasing, ghost activation and timeout, timed ASCII-art selection, configurable ambient intervals, trail collisions, fatal ghost contact, and option behavior. An actual Linux pseudo-terminal smoke test confirmed that timed ghost activation and ASCII warnings appear while rendering continues and the quit key works.

## Hacktoberfest contribution guidance

**Treat:** 2D territory-capture game using arrays and flood fill, plus toggleable horizontal speed boost.

**Trick:** Periodic wall-phasing ghost, with a nonblocking warning and five kinds of randomized decorative ASCII popups.

If submitting a pull request to an upstream GitHub project, check its contribution policy and scope first. The feature code is contained in `arena.c`, `arena.h`, and `main.c`, making it practical to review the gameplay changes separately from the preserved classic mode.

## License

MIT License. Original game Copyright (c) 2018 Breno G. de Oliveira. See `LICENSE`.
