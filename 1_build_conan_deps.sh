#!/bin/bash -e

# Installs to a subdirectory `id`

if [[ $(conan --version) == *" 1."* ]]; then
	conan install conanfile.txt -b missing -pr:b default -if id/cmake -of id/cmake -s build_type=Debug
	conan install conanfile.txt -b missing -pr:b default -if id/cmake -of id/cmake -s build_type=Release
else
	conan profile detect
	conan install conanfile.txt -b missing -pr:b default -of id/cmake -s build_type=Debug
	conan install conanfile.txt -b missing -pr:b default -of id/cmake -s build_type=Release
fi
