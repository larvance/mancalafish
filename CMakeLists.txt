cmake_minimum_required(VERSION 3.29)
project(mancalafish C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include_directories(.)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

add_executable(mancalafish src/engine.c src/player.c)
