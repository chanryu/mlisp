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