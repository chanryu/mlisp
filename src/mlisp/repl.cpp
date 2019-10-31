#include "repl.hpp"

#ifdef MLISP_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif
#include <sstream>

#include "print.hpp"

namespace mlisp {

namespace {
bool get_line(char const* prompt, std::string& line)
{
#ifdef MLISP_READLINE
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

int repl(std::shared_ptr<mll::Env> env)
{
    auto parser = mll::Parser{};

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
            break;
        }

        std::istringstream is(line);

        while (!is.eof()) {
            try {
                auto expr = parser.parse(is);
                if (!expr) {
                    break;
                }
                std::cout << eval(*expr, env) << std::endl;
            }
            catch (mll::ParseError& e) {
                std::cout << e.what() << std::endl;
            }
            catch (mll::EvalError& e) {
                std::cout << e.what() << std::endl;
            }
        }

        if (std::cin.eof()) {
            break;
        }
    }

    std::cout << std::endl;
    std::cout << "Bye." << std::endl;

    return parser.clean() ? 0 : -1;
}

} // namespace mlisp