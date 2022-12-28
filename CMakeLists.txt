cmake_minimum_required(VERSION 3.20)

project(EmscriptenTest)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

#add_definitions(-std=c++17)
set(CMAKE_CXX_STANDARD 20)

add_executable(VTPlayer
    VTPlayer.cpp
)

target_link_options(VTPlayer
    PRIVATE
        -sEXIT_RUNTIME=0
        -sINVOKE_RUN=0
        -sENVIRONMENT=worker
        -sBUILD_AS_WORKER
        -sMODULARIZE
        -sEXPORT_NAME=kk
        #"-sEXPORTED_RUNTIME_METHODS=['ccall']"
        -sEXPORT_ES6
)
