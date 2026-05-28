cmake_minimum_required(VERSION 3.10)
project(MdkCore)

set(PROJECT_SOURCE_FILES 
    src/utils/logger.cpp
    src/utils/arg_parser.cpp
    src/app/app.cpp
)

# Create library
add_library(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})

# Include directories
target_include_directories(MdkCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)