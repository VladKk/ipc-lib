[![CI](https://github.com/VladKk/ipc-lib/actions/workflows/ci.yml/badge.svg?branch=main&event=push)](https://github.com/VladKk/ipc-lib/actions/workflows/ci.yml)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)

# IPC Lib

## Summary
The IPC Lib is a simple demo/pet-project which demonstrates IPC capabilities of the Linux systems.\
It utilizes UNIX socket transport with framing, type-safe messaging, async receiving, shared memory and Qt bridge.

## Build
The project contains CMake presets. In order to configure and build the project you can use this command:
```
cmake --preset release && cmake --build --preset release
```
List of presets:
- gcc-debug
- clang-debug
- release
- asan-ubsan
- tsan

## Requirements
Here is the table of requirements for the project:

|           Tool            | Version |
|:-------------------------:|:-------:|
|            GCC            |   14+   |
|       Clang/libc++        |   19+   |
|           CMake           |  3.29+  |
|           Ninja           | 1.11.0+ |
| clang-format + clang-tidy |   19+   |
