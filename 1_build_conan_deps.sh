#!/bin/bash -e

# Installs to a subdirectory `id`

if [[ $(conan --version) == *" 1."* ]]; then
	conan install conanfile.txt -b missing -pr:b default -if id/cmake -of id/cmake -s build_type=Debug ${macos_option}
	conan install conanfile.txt -b missing -pr:b default -if id/cmake -of id/cmake -s build_type=Release ${macos_option}
else
	echo "Not implemented for Conan 2.x" >&2
fi
