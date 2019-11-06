#!/bin/bash

directories="src"
# Assume clang-format.sh resides in the same directory as clang-format-all.sh
scriptDirectory="$(cd "$(dirname "$0")" && pwd -P)"
find ${directories} -type f -print | xargs "${scriptDirectory}"/clang-format.sh
