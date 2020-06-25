cmake_minimum_required(VERSION 3.16)
project(Copper C)


set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(Copper
        src/main.c
        src/bitset.c
        src/bitset.h
        src/types.h
        src/graph.c
        src/graph.h
        src/graph6.c
        src/graph6.h
        src/vertice_queue.c
        src/vertice_queue.h)


target_link_libraries(Copper m pthread)
