#!/bin/bash -e

if [[ -z "$1" || -z "$2" ]]; then
  echo "Missing parameters: Debug|Release config|install|test|test-installed"
fi

readonly COMMAND="$1"
readonly CONFIG="$2"

if [[ "${COMMAND}" == "config" ]]; then
  cmake -S . -B _b-${CONFIG} -D CMAKE_INSTALL_PREFIX=_i -D CMAKE_BUILD_TYPE=${CONFIG}
fi

if [[ "${COMMAND}" == "install" ]]; then
  cmake --build _b-${CONFIG} --target install --config ${CONFIG} -j
fi

if [[ "${COMMAND}" == "test" ]]; then
  ctest --test-dir _b-${CONFIG} -C ${CONFIG} --output-on-failure
fi

if [[ "${COMMAND}" == "test-installed" ]]; then
  cmake -S test-installed-lib -B _bi-${CONFIG} -D CMAKE_BUILD_TYPE=${CONFIG} -D "CMAKE_PREFIX_PATH=$PWD/_d;$PWD/_i"
  cmake --build _bi-${CONFIG} --config ${CONFIG} -j
  ctest --test-dir _bi-${CONFIG} -C ${CONFIG} --output-on-failure
fi
