include(FetchContent)

FetchContent_Declare(
    libtrace
    GIT_REPOSITORY https://github.com/orbcode/libtrace.git
    GIT_TAG v0.1.0
)
FetchContent_MakeAvailable(libtrace)
