#!/bin/bash
cd "$(dirname "$0")"
source_dir=$(pwd)
build_dir="$1"

if [ ! -z "${build_dir}" ]; then
  rm -rdf "${build_dir}/*"
fi

cd "${build_dir}" || exit 1
emcmake cmake "${source_dir}"

emmake make -j4 Knight
