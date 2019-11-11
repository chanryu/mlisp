#!/bin/bash

ROOT=$(git rev-parse --show-toplevel)
BUILD="$ROOT/build"
mkdir -p "$BUILD" && pushd "$BUILD" && cmake -G "Unix Makefiles" .. && popd