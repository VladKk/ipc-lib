cmake_minimum_required(VERSION 3.28)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ipc_lib LANGUAGES CXX)

option(BUILD_TESTING "Build tests" ON)

if (BUILD_TESTING)
    enable_testing()
    include(cmake/gtest.cmake)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ipc STATIC
    include/ipc/error.hpp
)

# TODO: remove me when cpp sources will be added
set_target_properties(ipc PROPERTIES
        LINKER_LANGUAGE CXX
)

target_include_directories(ipc PUBLIC include)
target_compile_features(ipc PUBLIC cxx_std_23)
target_compile_options(ipc PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif ()
