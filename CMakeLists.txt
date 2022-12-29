cmake_minimum_required(VERSION 3.20)

project(EmscriptenTest)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

#add_definitions(-std=c++17)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src)
