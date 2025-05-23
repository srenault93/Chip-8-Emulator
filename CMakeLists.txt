cmake_minimum_required(VERSION 3.10)
project(Chip-8-Emulator)

set(CMAKE_CXX_STANDARD 17)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(Chip-8-Emulator src/main.cpp src/chip8.cpp)
target_link_libraries(Chip-8-Emulator ${SDL2_LIBRARIES})