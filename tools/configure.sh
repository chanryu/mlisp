#!/bin/bash

PROJECT=$(git rev-parse --show-toplevel)
BUILD="$PROJECT/build"
mkdir -p "$BUILD" && pushd "$BUILD" > /dev/null && cmake -G "Unix Makefiles" .. && popd > /dev/null