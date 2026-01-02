cmake_minimum_required(VERSION 3.22)
set(CMAKE_PROJECT_NAME reusable_synth_tests)
project(${CMAKE_PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${CMAKE_PROJECT_NAME})

add_subdirectory(${CMAKE_SOURCE_DIR}/../reusable_synth 
                 ${CMAKE_BINARY_DIR}/reusable_synth)