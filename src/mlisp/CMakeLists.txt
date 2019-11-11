# mlisp executable target
set(SOURCES
    src/mlisp/load.cpp
    src/mlisp/number.cpp
    src/mlisp/operators.cpp
    src/mlisp/parser.cpp
    src/mlisp/repl.cpp
    src/mlisp/string.cpp)
file(GLOB HEADERS src/mlisp/*.hpp)
add_executable(mlisp src/mlisp/main.cpp ${SOURCES} ${HEADERS})
set_target_properties(mlisp PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF)
target_link_libraries(mlisp PRIVATE mll)

# mlisp test target
file(GLOB TEST_SOURCES test/*.cpp)
add_executable(mlisp_test ${SOURCES} ${TEST_SOURCES})
set_target_properties(mlisp_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF)
target_link_libraries(mlisp_test PRIVATE mll)
