#!/bin/bash

# tool lookup list.
# the most version specific names go first.
CF_TOOL_NAMES=(\
    /usr/local/Cellar/llvm@7/7.1.0/bin/clang-format \
    /usr/local/Cellar/clang-format/2018-01-11/bin/clang-format \
    clang-format-7 \
    clang-format \
    )

# only parse files with the extensions in FILE_EXTS. Set to true or false.
# if false every changed file in the commit will be parsed with clang-format.
# if true only files matching one of the extensions are parsed with clang-format.
# PARSE_EXTS=true
PARSE_EXTS=true

# file types to parse. Only effective when PARSE_EXTS is true.
# FILE_EXTS=".c .h .cpp .hpp"
FILE_EXTS=".c .h .cpp .hpp .cc .hh .cxx .inl"
WHITELIST_DIRS="src"
BLACKLIST_DIRS="src/third-party"
# This will match any parent directory of a file
BLACKLIST_DIRS_ANYWHERE=""
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
  local path=$(dirname "$1")

  for dir in $BLACKLIST_DIRS; do
    [[ $1 == $dir* ]] && return 1;
  done

  while [ "$path" != "." ]
  do
    for dir in $BLACKLIST_DIRS_ANYWHERE; do
      [ $(basename "$path") == $dir ] && return 1;
    done
    path=$(dirname "$path")
  done

  for dir in $WHITELIST_DIRS; do
    [[ "$dir" == "$basedir" ]] && return 0;
  done

  return 1
}

STYLEARG="-style=file -assume-filename=${PWD}/.clang-format"

format_file() {
  file="${1}"
  # ignore file if we do check for file extensions and the file
  # does not match any of the extensions specified in $FILE_EXTS
  # or does not belong to a directory specified in WHITELIST_DIRS
  # or does belong to a directory specified in BLACKLIST_DIRS
  echo -n "  ${file}:"
  if $PARSE_EXTS && matches_extension "$file" && matches_directory "$file" && matches_file "$file"; then
    echo " applying clang-format"
    $CLANG_FORMAT -i ${STYLEARG} ${1} || {
      echo "Error executing '$CLANG_FORMAT'"
      exit 1
    }
  else
    echo " ignoring file"
  fi
}

# detect clang-format tool
for cf_tool in ${CF_TOOL_NAMES[@]}; do
  if (type ${cf_tool} && ${cf_tool} --version | grep "clang-format version 9") > /dev/null 2>&1; then
    CLANG_FORMAT=${cf_tool}
    break
  fi
done

if [ -z ${CLANG_FORMAT+x} ]; then
  echo >&2 "clang-format version 7 was not found"
  case $(uname) in
    Linux )
      echo >&2 "Run 'sudo apt install clang-format-7' to install"
      ;;
    Darwin )
      echo >&2 "Run 'brew install https://raw.githubusercontent.com/Homebrew/homebrew-core/f6841d0201ac6095abcc3bd82bc075316e2f94b1/Formula/clang-format.rb' to install"
      ;;
    * )
      # tell us how to get it installed on your machine.
      ;;
  esac
  exit 1
fi

case "${1}" in
  --about )
    echo "Runs clang-format on source files, as long as it's contained on a"
    echo "whitelisted directory and not contained on a blacklisted one."
    ;;
  * )
    if [ "$#" -lt 1 ]; then
      echo "You must supply a file list."
      exit 1
    fi
    for file in "$@"; do
      format_file "${file}"
    done
    ;;
esac
