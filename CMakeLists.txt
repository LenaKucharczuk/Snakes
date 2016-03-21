cmake_minimum_required(VERSION 3.3)
project(Snakes)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SPACECRAFT_SOURCE_FILES left.cpp)
add_executable(LEFT ${SPACECRAFT_SOURCE_FILES})

set(ENEMIES_SOURCE_FILES right.cpp)
add_executable(RIGHT ${ENEMIES_SOURCE_FILES})