cmake_minimum_required(VERSION 3.20)

project(EmscriptenTest)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

#add_definitions(-std=c++17)
set(CMAKE_CXX_STANDARD 20)

# Copying a file verbating can be done at configuration time via file(COPY ...),
# but that will fail to update the copied file at build time if the source changes.
# In order to do it at build time, it needs a custom command.
# This function creates a target to declare such files, which other targets can depend on.
function(add_copy_files TargetName)

    get_target_property(DestDir ${TargetName} RUNTIME_OUTPUT_DIRECTORY)

    set(DestinationFileList)
    foreach (SourceName ${ARGN})
        if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${SourceName}")
            set(SourcePath "${CMAKE_CURRENT_LIST_DIR}/${SourceName}")
            set(DestinationPath           "${DestDir}/${SourceName}")
        else()
            # TODO: Adapt...
            message(FATAL "add_copy_files: The source file ${SourceName} must be relative")
        endif()

        add_custom_command(
            OUTPUT          "${DestinationPath}"
            MAIN_DEPENDENCY "${SourcePath}"
            COMMAND         "${CMAKE_COMMAND}" -E copy_if_different "${SourcePath}" "${DestinationPath}"
        )
        list(APPEND DestinationFileList "${DestinationPath}")
    endforeach()

    # Creating a custom target allows the copy commands to be run.
    add_custom_target(${TargetName}.CopyFiles DEPENDS ${DestinationFileList})
    add_dependencies(${TargetName} ${TargetName}.CopyFiles)

endfunction()

add_executable(calc calc.cpp)

add_copy_files(calc index.html)

# main.cpp

