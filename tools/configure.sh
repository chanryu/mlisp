#!/bin/bash

PROJECT=$(git rev-parse --show-toplevel)
BUILD="$PROJECT/build"
rm -rf "$BUILD"
mkdir -p "$BUILD" && pushd "$BUILD" && cmake -G "Unix Makefiles" .. && popd