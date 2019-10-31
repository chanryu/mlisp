#include <cassert>
#include <sstream>

#if __has_include(<unistd.h>)
#include <unistd.h> // isatty
#define MLISP_EVAL_PIPED_STDIN 1
#else
#define MLISP_EVAL_PIPED_STDIN 0
#endif

#ifdef MLISP_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "eval.hpp"
#include "operators.hpp"
#include "print.hpp"

bool
get_line(char const* prompt, std::string& line)
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

int
repl(std::shared_ptr<mll::Env> env)
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

int main(int argc, char *argv[])
{
    auto env = mll::Env::create();

    set_primitive_procs(env);
    set_complementary_procs(env);

    set_number_procs(env);
    set_string_procs(env);
    set_symbol_procs(env);

    for (int i = 1; i < argc; ++i) {
        if (!eval_file(env, argv[i])) {
            return -1;
        }
    }

#if MLISP_EVAL_PIPED_STDIN
    bool is_stdin_piped = [] {
        return !isatty(fileno(stdin));
    }();
    if (is_stdin_piped) {
        return eval_stream(env, std::cin, std::cout) ? 0 : -1;
    }
#endif

    if (argc > 1) {
        return 0;
    }

    return repl(env);
}
