# CMakeList.txt : CMake project for chip8, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project("chip8")
set(SDL2_DIR "${CMAKE_CURRENT_LIST_DIR}/lib/SDL2-2.0.22")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Add source to this project's executable.
add_executable(chip8 "src/main.cpp" "src/Emulator.cpp" "src/Emulator.h" "src/Display.h" "src/Display.cpp" "src/Keypad.h" "src/Keypad.cpp" "src/Processor.h" "src/Processor.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET chip8 PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries("chip8" ${SDL2_LIBRARIES})
