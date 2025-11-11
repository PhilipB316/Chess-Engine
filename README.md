# TessMax

[![GitHub release](https://img.shields.io/github/release/PhilipB316/Chess-Engine.svg)](https://github.com/PhilipB316/Chess-Engine/releases/latest)
[![License](https://img.shields.io/github/license/PhilipB316/Chess-Engine.svg)](LICENSE)

TessMax is my first chess engine - a bitboard engine written in C. It uses a CLI for move input, and displays the position in a simple GUI.

## Features

### Engine Features

- Magic Bitboards
- Negamax Search
- Alpha-Beta Pruning
- Iterative Deepening
- Transposition Hash Table
- Move Repetition Hash Table (with linear probing)

### User Interface Features

- Engine search time setting
- Standard Algebraic Notation (SAN) input
- Colour selection (white or black)
- GUI display of the current position
- Move square and check highlighting
- Game logging to a text file with PGN format

## Building TessMax

### Prerequisites

- CMake 3.10 or higher
- gcc
- SDL2 - this can be installed via apt on some Linux systems with:

```sh
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```

### Build Instructions (for Linux)

```sh
git clone https://github.com/PhilipB316/Chess-Engine.git
cd Chess-Engine
mkdir build && cd build
cmake ..
make
```

The executable will be located in the `build` directory.

## Usage

Run TessMax from the command line from within the `build` directory:

```sh
./tessmax
```

The CLI instructions will guide you through the game. You can input moves in Standard Algebraic Notation (SAN) format.

The current position will be displayed in a simple GUI.

## Known 'Features'

- Engine prefers stalemate over checkmate if opponent has only a king remaining.

## Planned Features

- UCI Protocol support
- Previous move visualization in GUI
- Current position evaluation display
- Quiescence Search

## License

The license for this project can be found in the [LICENSE](LICENSE) file.
