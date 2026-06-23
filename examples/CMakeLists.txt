add_executable(ipc-examples
        req_rep.cpp
)

target_compile_options(ipc-examples PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(ipc-examples PRIVATE ipc)
