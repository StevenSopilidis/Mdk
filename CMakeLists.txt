cmake_minimum_required(VERSION 3.10)

project(Mdk)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Shared include directories
include_directories(
    src
    mdk/src
)

# Build core library
add_subdirectory(
    mdk
    "${CMAKE_CURRENT_BINARY_DIR}/mdk_build"
)

# CLI executable
add_executable(
    MdkCli
    src/cli/main.cpp
)

target_link_libraries(
    MdkCli
    MdkCore
)


# Daemon executable
add_executable(
    MdkDaemon
    src/daemon/main.cpp
)

target_link_libraries(
    MdkDaemon
    MdkCore
)