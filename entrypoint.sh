#!/bin/bash
cd "$(dirname "$0")" || exit 1
source_dir=$(pwd)
build_dir=/build

echo "--"
find "/emsdk/upstream/emscripten" | grep [.]so | grep sdl
echo "--"

mkdir "${build_dir}"
cd "${build_dir}" || exit 1
emcmake cmake "${source_dir}"