cmake_minimum_required(VERSION 3.20)

project(EmscriptenTest)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_INSTALL_PREFIX           "${CMAKE_BINARY_DIR}/install")

#add_definitions(-std=c++17)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src)

if ("${COMPILER_TOOLCHAIN}" STREQUAL "Emscripten")
    install(DIRECTORY "www/." DESTINATION ".")
    install(DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/." DESTINATION "js")
    install(DIRECTORY "js/." DESTINATION "js")
    install(DIRECTORY "C:/Users/jcab/Music/MODs" DESTINATION "MODs")
endif()
