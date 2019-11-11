#!/bin/bash

ROOT=$(git rev-parse --show-toplevel)
BUILD="$ROOT/build"
cmake --build "$BUILD"