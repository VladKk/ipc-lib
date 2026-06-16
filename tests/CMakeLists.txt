add_executable(ipc-tests
    dummy_test.cpp
)

target_compile_options(ipc-tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(ipc-tests PRIVATE gtest_main)

include(GoogleTest)
gtest_discover_tests(ipc-tests)
