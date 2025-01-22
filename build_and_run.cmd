@echo off
if exist build (
    rm build
)

mkdir build

cd build

cmake -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles" ..

cmake --build .

main.exe

REM for debug purposes
pause