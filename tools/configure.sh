#!/bin/bash

ROOT=$(git rev-parse --show-toplevel)
BUILD="$ROOT/build"
rm -rf "$BUILD"
mkdir -p "$BUILD" && pushd "$BUILD" && cmake -G "Unix Makefiles" .. && popd