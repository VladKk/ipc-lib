cmake_minimum_required(VERSION 3.29)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ipc_lib LANGUAGES CXX)

if (PROJECT_IS_TOP_LEVEL)
    enable_testing()
    include(cmake/gtest.cmake)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ipc INTERFACE
        include/ipc/addr.hpp
        include/ipc/error.hpp
        include/ipc/fd.hpp
        include/ipc/transport.hpp
        include/ipc/unix_socket.hpp
)

target_include_directories(ipc INTERFACE include)
target_compile_features(ipc INTERFACE cxx_std_23)

if (PROJECT_IS_TOP_LEVEL)
    add_subdirectory(tests)
endif ()
