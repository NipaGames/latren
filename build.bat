@echo off
rem Build type can be either Debug or Release
set BUILD_TYPE=Debug
echo [BUILD STARTED]

rem cmake -B build -G "Ninja" .
cmake %TARGET_OPTIONS% -DLATREN_BUILD_STANDALONE=ON -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -B build .
cd build
cmake --build . --config %BUILD_TYPE% || exit 1
echo [BUILD COMPLETE]
echo.
cd ..