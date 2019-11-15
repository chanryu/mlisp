cmake_minimum_required(VERSION 3.11)

project(mlisp)

################################################################################
# libraries
add_subdirectory(lib/mll)
add_subdirectory(lib/3rd-party/Catch2)

################################################################################
# mlisp executable target
set(MLISP_SOURCES
    src/mlisp/load.cpp
    src/mlisp/number.cpp
    src/mlisp/main.cpp
    src/mlisp/operators.cpp
    src/mlisp/parser.cpp
    src/mlisp/repl.cpp
    src/mlisp/string.cpp)
file(GLOB MLISP_HEADERS src/mlisp/*.hpp)
add_executable(mlisp ${MLISP_SOURCES} ${MLISP_HEADERS})
set_target_properties(mlisp PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF)
target_compile_options(mlisp PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Werror -Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:
        -W4>)
target_link_libraries(mlisp PRIVATE mll)

################################################################################
# mlisp test target
list(REMOVE_ITEM MLISP_SOURCES src/mlisp/main.cpp)
file(GLOB TEST_SOURCES src/test/*.cpp)
add_executable(mlisp_test ${MLISP_SOURCES} ${MLISP_HEADERS} ${TEST_SOURCES})
target_include_directories(mlisp_test PRIVATE src/mlisp)
set_target_properties(mlisp_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF)
target_link_libraries(mlisp_test PRIVATE mll Catch2)