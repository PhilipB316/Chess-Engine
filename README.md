# TessMax

TessMax is my first chess engine. It is a bitboard engine written in C.

## Features

### Engine Features

- Magic Bitboards
- Negamax Search
- Alpha-Beta Pruning
- Iterative Deepening

### User Interface Features

- Unicde board printing
- Engine search time setting
- Standard Algebraic Notation (SAN) input
- Colour selection (white or black)

## Building TessMax

### Prerequisites

- CMake 3.10 or higher
- gcc

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

Run TessMax from the command line:

```sh
./tessmax
```

The CLI instructions will guide you through the game. You can input moves in Standard Algebraic Notation (SAN) format.

## License

The license for this project can be found in the [LICENSE](LICENSE) file.
