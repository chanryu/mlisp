#!/bin/bash

FILE_EXTS=".c .h .cpp .hpp"
WHITELIST_DIRS="src lib"
BLACKLIST_DIRS="lib/3rd-party"
BLACKLIST_FILES=""

# check whether the given file matches any of the set extensions
matches_extension() {
  local filename=$(basename "$1")
  local extension=".${filename##*.}"
  local ext

  for ext in $FILE_EXTS; do
    [[ "$ext" == "$extension" ]] && return 0;
  done

  return 1
}

matches_file() {
  for file in $BLACKLIST_FILES; do
    [[ "$file" == "$1" ]] && return 1;
  done

  return 0
}

matches_directory() {
  local filename=$(basename "$1")
  local basedir=$(echo "$1" | awk -F'/' '{ print $1}')

  for dir in $BLACKLIST_DIRS; do
    [[ $1 == $dir* ]] && return 1;
  done

  for dir in $WHITELIST_DIRS; do
    [[ "$dir" == "$basedir" ]] && return 0;
  done

  return 1
}

CLANG_FORMAT=$(/usr/bin/which clang-format)
STYLEARG="-style=file -assume-filename=${PWD}/.clang-format"

format_file() {
  file="${1}"
  echo -n "  ${file}:"
  if matches_extension "$file" && matches_directory "$file" && matches_file "$file"; then
    echo " applying clang-format"
    $CLANG_FORMAT -i ${STYLEARG} ${1} || {
      echo "Error executing '$CLANG_FORMAT'"
      exit 1
    }
  else
    echo " ignoring file"
  fi
}

if [ ! -f ${CLANG_FORMAT} ]; then
  echo >&2 "Install clang-format"
  exit 1
fi

if [ "$#" -lt 1 ]; then
  echo "You must supply a file list."
  exit 1
fi

for file in "$@"; do
  format_file "${file}"
done