macro(download_googletest)
    include(FetchContent)

    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG 52eb8108c5bdec04579160ae17225d66034bd723
    )

    FetchContent_MakeAvailable(googletest)
endmacro()

download_googletest()
