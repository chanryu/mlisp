cmake_minimum_required(VERSION 3.11)

project(mlisp)

# mll library
add_library(mll STATIC 
    src/mll/custom.cpp
    src/mll/env.cpp
    src/mll/eval.cpp
    src/mll/node.cpp
    src/mll/parser.cpp
    src/mll/print.cpp
    src/mll/quote.cpp)
set_target_properties(mll PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF)
target_include_directories(mll PUBLIC src)

# mlisp excutable
add_executable(mlisp
    src/mlisp/load.cpp
    src/mlisp/main.cpp
    src/mlisp/number.cpp
    src/mlisp/operators.cpp
    src/mlisp/parser.cpp
    src/mlisp/repl.cpp
    src/mlisp/string.cpp)
set_target_properties(mlisp PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF)
target_link_libraries(mlisp PRIVATE mll)