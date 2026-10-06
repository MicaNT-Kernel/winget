cmake_minimum_required(VERSION 3.25)
project(winget VERSION 1.6.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler Warnings
if(MSVC)
    add_compile_options(/W4 /EHsc /utf-8 /wd4201 /wd4100)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif()

# Include Directory
include_directories(include)

# 1. Header-only Library Target
add_library(winget_lib INTERFACE)
target_include_directories(winget_lib INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 2. Standalone CLI Executable
add_executable(winget src/main.cpp)
target_link_libraries(winget PRIVATE winget_lib)

# 3. Unit Test Runner
enable_testing()
add_executable(winget_tests tests/test_winget.cpp)
target_link_libraries(winget_tests PRIVATE winget_lib)
add_test(NAME WinGetTest COMMAND winget_tests)

# Installation Rules
install(TARGETS winget DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)
