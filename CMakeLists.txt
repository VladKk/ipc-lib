cmake_minimum_required(VERSION 3.29)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ipc_lib LANGUAGES CXX)

if (PROJECT_IS_TOP_LEVEL)
    enable_testing()
    include(cmake/gtest.cmake)

    set(CMAKE_VERIFY_INTERFACE_HEADER_SETS ON)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ipc INTERFACE)
target_compile_features(ipc INTERFACE cxx_std_23)
target_sources(ipc INTERFACE
        FILE_SET HEADERS
        BASE_DIRS include
        FILES
            include/ipc/addr.hpp
            include/ipc/error.hpp
            include/ipc/fd.hpp
            include/ipc/transport.hpp
            include/ipc/unix_socket.hpp
)

if (PROJECT_IS_TOP_LEVEL)
    add_subdirectory(tests)
    add_subdirectory(examples)
endif ()
