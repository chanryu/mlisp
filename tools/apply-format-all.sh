#!/bin/bash

scriptDir="$(cd "$(dirname "$0")" && pwd -P)"
find "src" -type f -print | xargs "${scriptDir}"/apply-format.sh
find "lib" -type f -print | xargs "${scriptDir}"/apply-format.sh
