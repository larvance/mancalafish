cmake_minimum_required(VERSION 3.29)
project(mancalafish C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include_directories(./src/include)

find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    target_link_libraries(your_target PRIVATE OpenMP::OpenMP_CXX)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -DDEPTH=16 -mavx")

add_executable(mancalafish src/utils.c
        src/engine.c
        src/player.c)
