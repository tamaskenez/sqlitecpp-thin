#!/bin/bash -e

cmake -S test-installed-lib -B bdt -D CMAKE_BUILD_TYPE=Debug   -D "CMAKE_PREFIX_PATH=../i;../id"
cmake -S test-installed-lib -B brt -D CMAKE_BUILD_TYPE=Release -D "CMAKE_PREFIX_PATH=../i;../id"
cmake --build bdt --config Debug -j
cmake --build brt --config Release -j
ctest -C Debug --test-dir bdt -V
ctest -C Release --test-dir brt -V
