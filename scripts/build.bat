@echo off

rem Build type can be either Debug or Release
set BUILD_TYPE=Release
echo [BUILD STARTED]

rem cmake %TARGET_OPTIONS% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -B build .
cmake %TARGET_OPTIONS% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -B build . -G "Ninja"
cd build
cmake --build . --config %BUILD_TYPE% || exit 1
echo [BUILD COMPLETE]