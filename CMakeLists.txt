cmake_minimum_required(VERSION 3.20)
project(chess-engine C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 17)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O3 -std=c17 -march=native -flto -pthread -fomit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto -pthread -lm")

option(BUILD_WEB "Build for web (use web_main.c)" OFF)

include_directories(HEADER_FILES)

if(BUILD_WEB)
    set(MAIN_SRC "./src/web_main.c")
    add_compile_definitions(WEB_BUILD=1)
else()
    set(MAIN_SRC "./src/local_main.c")
    add_compile_definitions(WEB_BUILD=0)
endif()

file(GLOB SOURCES
    "./src/movefinding/movefinder.c"
    "./src/movefinding/lookuptables.c"
    "./src/movefinding/board.c"
    "./src/movefinding/memory.c"
    "./src/search/search.c"
    "./src/search/evaluate.c"
    "./src/search/hash_tables.c"
    "./src/interface/movedisplay.c"
    "./src/interface/ui.c"
    "./src/gui/gui.c"
    "./src/gui/log.c"
)

add_executable(tessmax ${MAIN_SRC} ${SOURCES})

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

target_link_libraries(tessmax ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES})
target_include_directories(tessmax PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})


# cmake_minimum_required(VERSION 3.20)
# project(chess-engine C)
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_C_STANDARD 17)
#
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O3 -std=c17 -march=native -flto -pthread -fomit-frame-pointer")
# # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -funroll-loops -fstrict-aliasing -ffast-math -falign-functions=32 -fipa-pta")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto -pthread -lm")
#
# include_directories(HEADER_FILES)
# file(GLOB MAIN
#     "./src/main.c"
#     "./src/movefinding/movefinder.c"
#     "./src/movefinding/lookuptables.c"
#     "./src/movefinding/board.c"
#     "./src/movefinding/memory.c"
#     "./src/search/search.c"
#     "./src/search/evaluate.c"
#     "./src/search/hash_tables.c"
#     "./src/interface/movedisplay.c"
#     "./src/interface/ui.c"
#     "./src/gui/gui.c"
#     "./src/gui/log.c"
#     )
#
# add_executable(tessmax ${MAIN})
#
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(SDL2 REQUIRED sdl2)
# pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
# pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
#
# target_link_libraries(tessmax ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES})
# target_include_directories(tessmax PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})

# cmake_minimum_required(VERSION 3.5)
# project(chess-engine C)
#
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_C_STANDARD 99)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -lm")
#
# include_directories(HEADER_FILES)
# file(GLOB MAIN
#     "./src/main.c"
#     "./src/movefinding/movefinder.c"
#     "./src/movefinding/lookuptables.c"
#     "./src/movefinding/board.c"
#     "./src/movefinding/memory.c"
#     "./src/search/search.c"
#     "./src/search/evaluate.c"
#     "./src/search/hash_tables.c"
#     "./src/ui/movedisplay.c"
#     "./src/ui/ui.c"
#     "./src/ui/gui.c"
#     "./src/ui/log.c"
#     )
#
# add_compile_options(-Wall -Wextra -O3 -std=c17)
#
# # Create the executable FIRST
# add_executable(tessmax ${MAIN})
#
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(SDL2 REQUIRED sdl2)
# pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
# pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
#
# target_link_libraries(tessmax ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES})
# target_include_directories(tessmax PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})
