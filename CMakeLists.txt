# CMakeLists.txt for Mandelbrot

cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

# Create an executable
add_executable(Mandelbrot
  main.cpp
  Mandelbrot.cpp
  )

# Set necessary compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -Werror -lSDL2")

# Create a compilation database
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Mandelbrot)
