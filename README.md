# Xonitix: Trick or Treat Edition (C11)

A Halloween-themed, two-dimensional terminal game built in **C11**, based on [Xonitix56](https://github.com/OneAltz/Xonitix56). Capture more than half the arena while dodging moving enemies and temporary wall-phasing ghosts.

This is the **Spooky ASCII edition** with a 2D arena, Ghost Mode, rotating Halloween art, and a horizontal speed boost. **It does not include the later Bugs Bunny meme feature.** The original one-line game is preserved as `xonitix-classic`.

The original Xonitix was created by Breno G. de Oliveira (2018) and released under the MIT License. See [LICENSE](LICENSE).

## Features

**Treat: 2D territory capture**

- Move in four directions and press **Space** while standing on captured territory (`#`) to start drawing.
- Draw a trail (`+`) through empty territory and reconnect to captured territory to close it.
- A flood-fill algorithm captures regions that contain no enemies. Each newly captured interior cell earns 100 points.
- Capture **more than 50%** of interior tiles to advance. Each new level adds an enemy, up to 12.
- Press **B** to toggle a **2x horizontal speed boost**. Intermediate tiles are checked for collisions even when boosting.

**Trick: Ghosts and Halloween surprises**

- By default, after about 20 seconds in a level, one enemy becomes a ghost (`G`) for 5 seconds.
- Ghosts can travel through claimed interior walls and end the game on contact with the player.
- A ghost warning appears beside the board for 2 seconds when Ghost Mode activates.
- Harmless ASCII popups randomly display a spider, bat, pumpkin, skull, or black cat for 1.2 seconds.
- The ASCII artwork never pauses gameplay or covers the board. Use `--ascii-interval N` to set a fixed display interval.

### Arena symbols

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

@ player   # claimed tile   + active trail   . enemy   G ghost
```

## System requirements

- A terminal with keyboard input and enough room for the arena plus the ASCII-art panel.
- A compiler supporting **C11**. No third-party game, graphics, or image libraries are required.
- Linux: GCC or Clang, and Make.
- macOS: Apple Command Line Tools (Clang and Make).
- Windows 10/11: MSYS2 UCRT64 GCC, plus Windows Terminal or PowerShell for playing.

**Terminal size:** For the requested board size, allow at least **24 additional columns** for the Halloween art and **3 additional rows** for status information. For smaller terminal windows, start with `-l 40 -r 12`. The game normally chooses a size based on your terminal, capped at 62 columns by 20 rows.

## Install and build

Download and extract the project ZIP, or clone the repository **after this C11 version has been committed to it**. Open a terminal **inside the folder containing `main.c`, `arena.c`, `terminal.c`, and `Makefile`** before running the build commands.

### Fedora Linux

Install the compiler and Make:

```bash
sudo dnf install gcc make
```

Build, test, and launch:

```bash
make clean
make
make test
./xonitix
```

### Ubuntu, Debian, Linux Mint, and Pop!_OS

```bash
sudo apt update
sudo apt install build-essential
make clean
make
make test
./xonitix
```

### Arch Linux and Manjaro

```bash
sudo pacman -S --needed base-devel
make clean
make
make test
./xonitix
```

### openSUSE

```bash
sudo zypper install gcc make
make clean
make
make test
./xonitix
```

### Alpine Linux

```bash
sudo apk add build-base
make clean
make
make test
./xonitix
```

### Other Linux distributions

Install a **C11 compiler** (such as GCC or Clang) and **Make** using your distribution's package manager. Then, from the project directory:

```bash
make clean
make
make test
./xonitix
```

If Make is unavailable, compile the 2D version directly:

```bash
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic main.c arena.c terminal.c -o xonitix
./xonitix
```

### macOS (Intel and Apple silicon)

1. Open **Terminal**.
2. Install Apple's developer command-line tools if needed:

   ```bash
   xcode-select --install
   ```

3. Once the installation completes, verify the compiler and build tools:

   ```bash
   clang --version
   make --version
   ```

4. Change to the extracted project directory, then build and play:

   ```bash
   make clean
   make
   make test
   ./xonitix
   ```

You do **not** need the full Xcode application or Homebrew for this build. The included Makefile uses the system C compiler, which is typically Clang on macOS.

### Windows 10/11 (MSYS2 UCRT64 GCC)

**Install the compiler:**

1. Install [MSYS2](https://www.msys2.org/).
2. Launch **MSYS2 UCRT64** from the Start menu, not the plain MSYS2 shell.
3. Update the package database and installed packages:

   ```bash
   pacman -Syu
   ```

   If asked to close the terminal, reopen **MSYS2 UCRT64** and run `pacman -Syu` again until updates complete.

4. Install GCC and check it:

   ```bash
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc
   gcc --version
   ```

**Build the game in MSYS2 UCRT64:**

In the MSYS2 UCRT64 terminal, navigate to the extracted C11 project folder. For example, if the ZIP was extracted to your Windows Downloads folder:

```bash
cd /c/Users/YOUR_USERNAME/Downloads/xonitix-trick-or-treat
```

Replace `YOUR_USERNAME` and the folder location as needed. Compile the 2D game:

```bash
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic main.c arena.c terminal.c -o xonitix.exe
```

**Play in Windows Terminal or PowerShell**, from the same folder:

```powershell
cd "$HOME\Downloads\xonitix-trick-or-treat"
.\xonitix.exe
```

Test Ghost Mode and artwork quickly:

```powershell
.\xonitix.exe -l 40 -r 12 --ghost-interval 4 --ghost-duration 3 --ascii-interval 2
```

**Optional Windows tests:** Run these in **MSYS2 UCRT64**, from the project folder:

```bash
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic tests/test_arena.c arena.c -o test-arena.exe
./test-arena.exe

gcc -std=c11 -O2 -Wall -Wextra -Wpedantic tests/test_game.c game.c -o test-game.exe
./test-game.exe
```

The provided Makefile uses Unix shell commands, so the documented Windows build uses GCC directly instead of `make`. Windows-specific console code is included but **has not been runtime-tested on Windows**.

## Controls

| Input | Action |
| --- | --- |
| Left / Right arrows, or A / D | Move horizontally |
| Up / Down arrows, or W / S | Move vertically |
| Space | Arm territory drawing while standing on `#` |
| B | Toggle 2x left/right movement speed |
| X | Stop moving |
| Q | Quit |

Movement continues in the chosen direction until changed or stopped. Press Space on claimed territory, cross unclaimed space, and return to claimed territory to complete a capture. Touching your own unfinished trail or an enemy can end the game.

## Command-line options

On Linux and macOS, use `./xonitix`. On Windows PowerShell, replace this with `.\xonitix.exe`.

| Option | Effect | Values / default |
| --- | --- | --- |
| `-l N` | Arena width in columns | 12 to 120 |
| `-r N` | Arena height in rows | 8 to 45 |
| `--ghost-interval N` | Delay between ghost activations | 1 to 300 seconds; default 20 |
| `--ghost-duration N` | Duration of active Ghost Mode | 1 to 300 seconds; default 5 |
| `--ascii-interval N` | Fixed interval between harmless art popups | 1 to 300 seconds; default randomized |
| `--no-ghost` | Disable dangerous Ghost Mode | Flag |
| `--no-ascii` | Hide Halloween art and ghost warning artwork | Flag |
| `-s` | Clear the game board on exit | Flag |
| `-h`, `--help` | Print built-in help | Flag |

**Examples (Linux/macOS):**

```bash
./xonitix                                              # Default gameplay
./xonitix -l 60 -r 20                                 # Set width and height
./xonitix --no-ghost --ascii-interval 2               # Harmless art demo
./xonitix --ghost-interval 4 --ghost-duration 3       # Frequent ghost attacks
./xonitix -l 40 -r 12 --ghost-interval 4 --ascii-interval 2
./xonitix --no-ascii                                  # Game without popups
```

## Original one-line mode

The classic C port is still available. On Linux/macOS, `make` also builds `xonitix-classic`:

```bash
./xonitix-classic
./xonitix-classic -h
```

On Windows, compile it using MSYS2 UCRT64:

```bash
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic classic.c game.c terminal.c -o xonitix-classic.exe
```

Then run `xonitix-classic.exe` from PowerShell. Classic supports `-l N`, `-q`, `-s`, and `-h`, but **does not include** the 2D arena, Ghost Mode, horizontal boost, or rotating ASCII art.

## Troubleshooting

- **`undefined reference to arena_init` or similar errors:** Your build is using the old source list (`main.c game.c terminal.c`). Use this project's Makefile, which links `main.c arena.c terminal.c`, and run `make clean && make`.
- **`No such file or directory` for `arena.c`:** Make sure you extracted the complete Spooky ASCII edition and are in the directory with `arena.c` and `Makefile`.
- **`Clock skew detected` on Linux/macOS:** Files may have future timestamps after extraction. Run `touch Makefile *.c *.h tests/*.c`, then `make clean && make`.
- **Terminal too small or artwork clipped:** Maximize the terminal, reduce font size, or try `./xonitix -l 40 -r 12`. The art needs extra columns on the right.
- **`Unable to start game. Use an interactive terminal`:** Run the game inside a normal interactive terminal, not a redirected shell or IDE output panel. On Windows, use Windows Terminal or PowerShell rather than the MSYS2 MinTTY window.
- **`gcc: command not found` on Windows:** Open MSYS2 UCRT64, install `mingw-w64-ucrt-x86_64-gcc`, and build there.
- **PowerShell does not launch `xonitix.exe`:** Use `.\xonitix.exe` from the directory containing the compiled file.
- **Terminal display does not recover after an unexpected interruption (Linux/macOS):** Try `stty sane` followed by `printf '\033[?25h'`.

## Project files

| File | Purpose |
| --- | --- |
| `main.c` | Main 2D loop, options, rendering, Halloween art |
| `arena.c`, `arena.h` | Arena state, capture logic, movement, boost, ghost logic |
| `terminal.c`, `terminal.h` | Input, screen measurements, clock, platform support |
| `classic.c`, `game.c`, `game.h` | Original single-line game port |
| `tests/test_arena.c`, `tests/test_game.c` | Automated game-engine tests |
| `Makefile` | Builds the two games and their tests on Unix-like platforms |
| `LICENSE` | MIT license and attribution |

## Testing and platform status

The C11 project has been compiled and tested on **Linux**. Both automated test suites pass. The macOS build instructions use Apple's standard Clang toolchain, but macOS gameplay is **not yet verified**. Windows console handling is implemented, but the Windows build and gameplay are **not yet verified on Windows**. Testing contributions are welcome.

## Hacktoberfest Trick or Treat contribution

- **Trick:** Timed wall-phasing ghost, ghost warning, and five randomized ASCII Halloween decorations.
- **Treat:** Fully playable 2D territory-capture mode using breadth-first flood fill, plus optional horizontal speed boost.

## License

MIT License. Original Xonitix copyright (c) 2018 Breno G. de Oliveira. See [`LICENSE`](LICENSE).
