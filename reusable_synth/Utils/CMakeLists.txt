cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_sources(ReusableSynth
    # PRIVATE 
    # put source files here
    PUBLIC
        FILE_SET HEADERS 
        FILES 
            noncopyable.hpp
)