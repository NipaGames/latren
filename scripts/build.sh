#!/bin/bash

# Build type can be either Debug or Release
BUILD_TYPE=Release
echo \[BUILD STARTED\]

# cmake %TARGET_OPTIONS% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -B build .
cmake %TARGET_OPTIONS% -DCMAKE_BUILD_TYPE=$BUILD_TYPE -B build . -G "Ninja"
cd build
cmake --build . --config %BUILD_TYPE% || exit 1
echo \[BUILD COMPLETE\]