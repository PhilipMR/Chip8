cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(chip8)
add_executable(chip8
    src/main.cpp
    src/chip8.cpp 
    src/display.cpp
)
target_compile_options(chip8 PRIVATE /W3 /Od)
include_directories(external/include)
target_link_libraries(chip8 PRIVATE
    ${CMAKE_SOURCE_DIR}/external/lib/x64/SDL2.lib
    ${CMAKE_SOURCE_DIR}/external/lib/x64/SDL2main.lib
    ${CMAKE_SOURCE_DIR}/external/lib/x64/SDL2_ttf.lib
)