cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Mdk)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT_SOURCE_FILES
    src/main.cpp
)

include_directories(src)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})

add_subdirectory(mdk "${CMAKE_CURRENT_BINARY_DIR}/mdk_build")
include_directories(mdk/src)

target_link_libraries(${PROJECT_NAME}
    MdkCore
)