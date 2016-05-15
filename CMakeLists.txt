cmake_minimum_required(VERSION 2.8.7)
project(game-of-life)

set(LIFE_VERSION_MAJOR 0)
set(LIFE_VERSION_MINOR 1)

add_executable(game_of_life
  src/main.c
  src/life.c
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall -Wextra -pedantic -march=native -fopenmp")
