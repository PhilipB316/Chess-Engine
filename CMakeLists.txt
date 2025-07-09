cmake_minimum_required(VERSION 3.5)
project(chess-engine C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
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
    "./src/ui/gui.c"
    )

add_compile_options(-Wall -Wextra -O3 -std=c17)

# Create the executable FIRST
add_executable(tessmax ${MAIN})

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

target_link_libraries(tessmax ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
target_include_directories(tessmax PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})