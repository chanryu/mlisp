
#include <mll/env.hpp>
#include <mll/eval.hpp>
#include <mll/print.hpp>

#include "load.hpp"
#include "operators.hpp"
#include "parser.hpp"
#include "repl.hpp"

#include <iostream>

#if __has_include(<unistd.h>)
#include <unistd.h> // isatty
#define MLISP_EVAL_PIPED_STDIN 1
#else
#define MLISP_EVAL_PIPED_STDIN 0
#endif

int main(int argc, char* argv[])
{
    auto env = mll::Env::create();

    mlisp::set_primitive_procs(*env);
    mlisp::set_complementary_procs(*env);

    mlisp::set_number_procs(*env);
    mlisp::set_string_procs(*env);
    mlisp::set_symbol_procs(*env);

    for (int i = 1; i < argc; ++i) {
        if (!mlisp::load_file(*env, argv[i])) {
            return -1;
        }
    }

#if MLISP_EVAL_PIPED_STDIN
    bool is_stdin_piped = [] {
        return !isatty(fileno(stdin));
    }();
    if (is_stdin_piped) {
        try {
            mlisp::Parser parser;
            while (true) {
                auto expr = parser.parse(std::cin);
                if (!expr.has_value()) {
                    break;
                }
                mll::print(std::cout, mll::eval(*expr, *env));
                std::cout << '\n';
            }
            return parser.clean() ? 0 : -1;
        }
        catch (mll::ParseError& e) {
            std::cout << e.what() << '\n';
            return -1;
        }
        catch (mll::EvalError& e) {
            std::cout << e.what() << '\n';
            return -1;
        }
    }
#endif

    if (argc > 1) {
        return 0;
    }

    return mlisp::repl(*env);
}
