cmake_minimum_required(VERSION 3.5)
project(chess-engine C)
set(CMAKE_C_CSTANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -lm")
include_directories(HEADER_FILES)
file(GLOB MAIN
    "./src/main.c"
    "./src/movefinding/movefinder.c"
    "./src/movefinding/lookuptables.c"
    "./src/movefinding/board.c"
    )
add_executable(executable ${MAIN})
