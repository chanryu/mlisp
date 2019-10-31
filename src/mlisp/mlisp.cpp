#include <cassert>

#if __has_include(<unistd.h>)
#include <unistd.h> // isatty
#define MLISP_EVAL_PIPED_STDIN 1
#else
#define MLISP_EVAL_PIPED_STDIN 0
#endif

#include <mll/mll.hpp>

#include "eval.hpp"
#include "operators.hpp"
#include "repl.hpp"

int main(int argc, char *argv[])
{
    auto env = mll::Env::create();

    mlisp::set_primitive_procs(*env);
    mlisp::set_complementary_procs(*env);

    mlisp::set_number_procs(*env);
    mlisp::set_string_procs(*env);
    mlisp::set_symbol_procs(*env);

    for (int i = 1; i < argc; ++i) {
        if (!mlisp::eval_file(*env, argv[i])) {
            return -1;
        }
    }

#if MLISP_EVAL_PIPED_STDIN
    bool is_stdin_piped = [] {
        return !isatty(fileno(stdin));
    }();
    if (is_stdin_piped) {
        return mlisp::eval_stream(*env, std::cin, std::cout) ? 0 : -1;
    }
#endif

    if (argc > 1) {
        return 0;
    }

    return mlisp::repl(*env);
}
