cmake_minimum_required(VERSION 3.22)
set(CMAKE_PROJECT_NAME reusable_synth_tests)
project(${CMAKE_PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  ${CMAKE_PROJECT_NAME}
)

add_subdirectory(mocks)
add_subdirectory(tests)

include_directories(${CMAKE_SOURCE_DIR}/../reusable_synth)
link_directories(${CMAKE_SOURCE_DIR}/../reusable_synth)
find_library(NAMES ReusableSynth PATH ${CMAKE_SOURCE_DIR}/../reusable_synth/build/Debug)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ..
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
  GTest::gtest_main
  GTest::gmock
  GTest::gmock_main
  ReusableSynth
)

include(GoogleTest)
gtest_discover_tests(${CMAKE_PROJECT_NAME})

# add_subdirectory(${CMAKE_SOURCE_DIR}/../reusable_synth 
#                  ${CMAKE_BINARY_DIR}/reusable_synth)