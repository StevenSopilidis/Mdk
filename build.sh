#!/bin/bash
rm -rf ./build
mkdir -p build
cd build

# Specify GCC 13 explicitly
cmake -DCMAKE_C_COMPILER=/usr/bin/gcc-13 \
      -DCMAKE_CXX_COMPILER=/usr/bin/g++-13 \
      ..

cmake --build . -j