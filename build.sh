#!/bin/bash

rm -rf ./build
mkdir -p build
cd build

cmake \
    -DCMAKE_C_COMPILER=/usr/bin/gcc-14 \
    -DCMAKE_CXX_COMPILER=/usr/bin/g++-14 \
    ..

cmake --build . -j