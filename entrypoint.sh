#!/bin/bash
cd "$(dirname "$0")" || exit 1
source_dir=$(pwd)
build_dir=/build

mkdir "${build_dir}"
cd "${build_dir}" || exit 1
emcmake cmake "${source_dir}"

emmake make Knight