# TessMax

TessMax is my first chess engine. It is a bitboard engine written in C. It uses a CLI for move input, and displays the position in a simple GUI.

## Features

### Engine Features

- Magic Bitboards
- Negamax Search
- Alpha-Beta Pruning
- Iterative Deepening

### User Interface Features

- Engine search time setting
- Standard Algebraic Notation (SAN) input
- Colour selection (white or black)
- GUI display of the current position

## Building TessMax

### Prerequisites

- CMake 3.10 or higher
- gcc
- SDL2 - this can be installed via apt with:

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

## License

The license for this project can be found in the [LICENSE](LICENSE) file.
