#!/bin/bash
cd "$(dirname "$0")"

mkdir ./build 2>/dev/null

docker build -t build_with_emscripten .

docker run --rm -u $(id -u):$(id -g) -v $(pwd):/mount:ro -v $(pwd)/build:/build build_with_emscripten

