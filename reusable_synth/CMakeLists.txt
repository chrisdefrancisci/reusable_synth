cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ReusableSynth)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    # Add user defined include paths
    ..
    ${CMAKE_SOURCE_DIR}
)

add_subdirectory(Hardware)