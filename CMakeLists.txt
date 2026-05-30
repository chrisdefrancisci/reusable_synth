cmake_minimum_required(VERSION 3.22)
project(ReusableSynth VERSION 0.1.0)

enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(ReusableSynthTests)
add_library(ReusableSynthInterface)
set_target_properties(ReusableSynthInterface PROPERTIES LINKER_LANGUAGE CXX)

target_compile_options(ReusableSynthTests PRIVATE -Wall)

target_sources(ReusableSynthTests
    PUBLIC
        FILE_SET HEADERS 
        BASE_DIRS 
            ${CMAKE_CURRENT_SOURCE_DIR}

)
target_sources(ReusableSynthInterface
    PUBLIC
        FILE_SET HEADERS 
        BASE_DIRS 
            ${CMAKE_CURRENT_SOURCE_DIR}

)

add_subdirectory(tests)
add_subdirectory(reusable_synth)

target_link_libraries(
  ReusableSynthTests
  PRIVATE
  GTest::gtest_main
  GTest::gmock
  GTest::gmock_main
  ReusableSynthInterface
)

include(GoogleTest)
gtest_discover_tests(ReusableSynthTests)
