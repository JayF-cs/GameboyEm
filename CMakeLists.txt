cmake_minimum_required(VERSION 3.10)
project(Emulator CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 REQUIRED)

add_executable(EMULATOR
    src/main.cpp
    src/cpu.cpp
    src/Bus.cpp
    src/Cartridge.cpp
    src/ppu.cpp
    src/render.cpp
    src/Timer.cpp
    src/input.cpp
)

target_include_directories(EMULATOR PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(EMULATOR PRIVATE mingw32 ${SDL2_LIBRARIES})