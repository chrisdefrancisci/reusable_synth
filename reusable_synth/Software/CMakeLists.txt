cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE 
        task.cpp
    PUBLIC
        FILE_SET HEADERS 
        BASE_DIRS 
            ${CMAKE_CURRENT_SOURCE_DIR}
        FILES 
            task.hpp
)