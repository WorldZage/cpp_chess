#!/bin/bash
echo "Building"
cmake -S . -B build
echo "Compiling"
cmake --build build
echo "Run"
build/cpp_chess


