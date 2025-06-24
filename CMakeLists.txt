cmake_minimum_required(VERSION 3.5)
project(chess-engine C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_CSTANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -lm")

include_directories(HEADER_FILES)
file(GLOB MAIN
    "./src/main.c"
    "./src/movefinding/movefinder.c"
    "./src/movefinding/lookuptables.c"
    "./src/movefinding/board.c"
    "./src/movefinding/memory.c"
    "./src/search/search.c"
    "./src/search/evaluate.c"
    "./src/ui/movedisplay.c"
    "./src/ui/ui.c"
    )
add_compile_options(-Wall -Wextra -Wpedantic -O3)

add_executable(executable ${MAIN})
