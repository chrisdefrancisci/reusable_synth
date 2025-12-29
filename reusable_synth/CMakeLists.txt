cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable CMake support for ASM and C languages
enable_language(C CXX ASM)

add_library(ReusableSynth)

set_target_properties(ReusableSynth PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    # Add user defined include paths
    ..
    ${CMAKE_SOURCE_DIR}
)

add_subdirectory(Hardware)