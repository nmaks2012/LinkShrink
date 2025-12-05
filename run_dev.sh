#!/bin/bash
set -e

cmake -B ./build/release -S . -DCMAKE_BUILD_TYPE=Debug

cmake --build ./build/release -- -j8

./build/release/linkshrink-app --config configs/static_config.yaml --config_vars configs/config_vars.yaml