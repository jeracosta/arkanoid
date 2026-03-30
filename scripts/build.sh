#!/bin/sh

target="$1"

if [ ! -d build ]; then
  cmake -B build -DCMAKE_BUILD_TYPE=Release
fi

cmake --build build ${target:+--target "$target"} -j
