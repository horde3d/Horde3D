#!/bin/bash

docker build -t build_with_emscripten .

docker run -v $(pwd):/mount build_with_emscripten:ro -u $(id -u):$(id -g)
