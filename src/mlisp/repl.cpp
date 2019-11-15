#include "repl.hpp"

#include "parser.hpp"

#include <mll/eval.hpp>
#include <mll/node.hpp>
#include <mll/print.hpp>

#if __has_include(<linenoise/linenoise.h>)
#include <linenoise/linenoise.h>
#define MLISP_LINENOISE 1
#else
#define MLISP_LINENOISE 0
#endif

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace mlisp {

namespace {
bool get_line(char const* prompt, std::string& line)
{
#if MLISP_LINENOISE
    if (auto rp = ::linenoise(prompt)) {
        std::unique_ptr<char, decltype(::free)*> sp{rp, ::free};
        line = sp.get();
        if (!line.empty()) {
            linenoiseHistoryAdd(line.c_str());
        }
        return true;
    }
#else
    std::cout << prompt;
    if (std::getline(std::cin, line)) {
        return true;
    }
#endif
    return false;
}
} // namespace

int repl(mll::Env& env)
{
    mlisp::Parser parser;

    while (true) {
        char const* prompt;
        if (parser.clean()) {
            prompt = "mlisp> ";
        }
        else {
            prompt = "...... ";
        }

        std::string line;
        if (!get_line(prompt, line)) {
            std::cout << '\n';
            break;
        }

        std::istringstream is{line};
        while (!is.eof()) {
            try {
                auto expr = parser.parse(is);
                if (!expr.has_value()) {
                    break;
                }
                auto value = mll::eval(*expr, env);
                std::cout << "=====> ";
                mll::print(std::cout, value);
                std::cout << '\n';
            }
            catch (mll::ParseError& e) {
                std::cout << e.what() << '\n';
            }
            catch (mll::EvalError& e) {
                std::cout << e.what() << '\n';
            }
        }
    }

    std::cout << "Moriturus te saluto.\n";

    return parser.clean() ? 0 : -1;
}

} // namespace mlisp