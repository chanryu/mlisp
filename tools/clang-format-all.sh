#!/bin/bash

scriptDir="$(cd "$(dirname "$0")" && pwd -P)"
find "src" -type f -print | xargs "${scriptDir}"/clang-format.sh
