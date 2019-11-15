add_library(linenoise STATIC src/linenoise.c)
target_include_directories(linenoise INTERFACE include PRIVATE include/linenoise)