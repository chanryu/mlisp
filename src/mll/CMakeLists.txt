# mll library target
set(SOURCES 
    src/mll/custom.cpp
    src/mll/env.cpp
    src/mll/eval.cpp
    src/mll/node.cpp
    src/mll/parser.cpp
    src/mll/print.cpp
    src/mll/quote.cpp)
file(GLOB HEADERS src/mll/*.hpp)
add_library(mll STATIC ${SOURCES} ${HEADERS})
set_target_properties(mll PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF)
target_include_directories(mll PUBLIC src)

# mll test target
file(GLOB TEST_SOURCES test/*.cpp)
add_executable(mll_test ${TEST_SOURCES})
set_target_properties(mll_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF)
target_link_libraries(mll_test PRIVATE mll)
