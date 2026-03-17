#!/bin/bash
set -e

rm -rf build
mkdir build
cd build
cmake .. -G "MinGW Makefiles" \
  -DCMAKE_CXX_COMPILER="C:/msys64/ucrt64/bin/g++.exe" \
  -DCMAKE_MAKE_PROGRAM="C:/msys64/ucrt64/bin/mingw32-make.exe"
C:/msys64/ucrt64/bin/mingw32-make.exe
