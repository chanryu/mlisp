#!/bin/bash

PROJECT=$(git rev-parse --show-toplevel)
BUILD="$PROJECT/build"
mkdir -p "$BUILD" && pushd "$BUILD" && cmake -G "Unix Makefiles" .. && popd