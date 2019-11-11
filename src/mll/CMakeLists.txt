set(SOURCES 
    src/mll/custom.cpp
    src/mll/env.cpp
    src/mll/eval.cpp
    src/mll/node.cpp
    src/mll/parser.cpp
    src/mll/print.cpp
    src/mll/quote.cpp)
add_library(mll STATIC ${SOURCES})
set_target_properties(mll PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF)
target_include_directories(mll PUBLIC src)