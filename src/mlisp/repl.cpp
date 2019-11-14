#include "repl.hpp"

#include "parser.hpp"

#include <mll/eval.hpp>
#include <mll/node.hpp>
#include <mll/print.hpp>

#if __has_include(<readline/readline.h>)
#define MLISP_READLINE 1
#include <readline/history.h>
#include <readline/readline.h>
#else
#define MLISP_READLINE 0
#endif
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace mlisp {

namespace {
bool get_line(char const* prompt, std::string& line)
{
#if MLISP_READLINE
    static bool once = true;
    if (once) {
        once = false;
        // By default readline does filename completion. We disable this
        // by asking readline to just insert the TAB character itself.
        rl_bind_key('\t', rl_insert);
    }

    auto buf = readline(prompt);
    if (buf) {
        line = buf;
        free(buf);
        if (!line.empty()) {
            add_history(line.c_str());
        }
        line.push_back('\n');
        return true;
    }
#else
    std::cout << prompt;
    if (std::getline(std::cin, line)) {
        line.push_back('\n');
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