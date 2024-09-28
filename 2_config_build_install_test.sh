#!/bin/bash -e

if [[ -z "$1" || "$1" == "config" ]]; then
	cmake -S . -B bd -D CMAKE_BUILD_TYPE=Debug -D BUILD_TESTING=ON -D CMAKE_INSTALL_PREFIX=i
	cmake -S . -B br -D CMAKE_BUILD_TYPE=Release -D BUILD_TESTING=ON -D CMAKE_INSTALL_PREFIX=i
fi

if [[ -z "$1" || "$1" == "build_install" ]]; then
	cmake --build bd --config Debug --target install -j
	cmake --build br --config Release --target install -j
fi

if [[ -z "$1" || "$1" == "test" ]]; then
	ctest -C Debug --test-dir bd -V
	ctest -C Release --test-dir br -V
fi
