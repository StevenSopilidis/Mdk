cmake_minimum_required(VERSION 3.10)
project(MdkCore)

set(PROJECT_SOURCE_FILES 
    src/utils/logger.cpp
)

# Include directories
include_directories(
    src/utils
)

# Create library
add_library(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})