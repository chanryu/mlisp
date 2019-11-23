# mll library target
set(SOURCES 
    src/mll/custom.cpp
    src/mll/env.cpp
    src/mll/eval.cpp
    src/mll/list.cpp
    src/mll/node.cpp
    src/mll/parser.cpp
    src/mll/print.cpp
    src/mll/proc.cpp
    src/mll/quote.cpp
    src/mll/symbol.cpp)
file(GLOB HEADERS src/mll/*.hpp)
add_library(mll STATIC ${SOURCES} ${HEADERS})
set_target_properties(mll PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF)
target_compile_options(mll PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Werror -Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:
        -W4>)
target_include_directories(mll PUBLIC src)

# mll test target
file(GLOB TEST_SOURCES test/*.cpp)
add_executable(mll_test ${TEST_SOURCES})
set_target_properties(mll_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF)
target_link_libraries(mll_test PRIVATE mll Catch2)
