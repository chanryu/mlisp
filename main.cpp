#include <cassert>
#include <fstream>
#include <sstream>

#include "mlisp.hpp"

using namespace mlisp;

bool is_symbol(Object obj)
{
    return !!to_symbol(obj);
}

Cons to_list(Object obj, char const* cmd)
{
    auto list = to_cons(obj);
    if (!list) {
        throw EvalError(cmd + (": " + std::to_string(obj)) + " is not a list.");
    }
    return *list;
}

Number to_number(Object obj, char const* cmd)
{
    auto number = to_number(obj);
    if (!number) {
        throw EvalError(cmd + (": " + std::to_string(obj)) + " is not a number.");
    }
    return *number;
}

Cons to_formal_args(Object obj, char const* cmd)
{
    auto args = to_list(obj, cmd);

    // validate args (must be list of symbols)
    for (auto c = args; c; c = cdr(c)) {
        if (!is_symbol(car(c))) {
            throw EvalError(cmd + (": " + std::to_string(car(c))) + " is not a symbol");
        }
    }

    return args;
}

Object cadr(Cons list)
{
    return car(cdr(list));
}

EnvPtr build_env()
{
    auto env = make_env(nullptr);

    set(env, "car", make_proc([] (Cons args, EnvPtr env) {
        if (cdr(args)) {
            throw EvalError("car: too many args given");
        }
        return car(to_list(eval(car(args), env), "car"));
    }));

    set(env, "cdr", make_proc([] (Cons args, EnvPtr env) {
        if (cdr(args)) {
            throw EvalError("cdr: too many args given");
        }
        return cdr(to_list(eval(car(args), env), "cdr"));
    }));

    set(env, "cons", make_proc([] (Cons args, EnvPtr env) {
        if (!cdr(args)) {
            throw EvalError("cons: not enough args");
        }

        auto head = eval(car(args), env);
        auto tail = to_list(eval(cadr(args), env), "cons");

        return make_cons(head, tail);
    }));

    set(env, "list", make_proc([] (Cons args, EnvPtr env) {
        std::vector<Object> objs;
        for (; args; args = cdr(args)) {
            objs.push_back(eval(car(args), env));
        }

        Cons list;
        while (!objs.empty()) {
            list = make_cons(objs.back(), list);
            objs.pop_back();
        }
        return list;
    }));

    set(env, "set", make_proc([] (Cons args, EnvPtr env) {
        if (!args || !cdr(args)) {
            throw EvalError("set: too few parameters");
        }
        if (cdr(cdr(args))) {
            throw EvalError("set: too many parameters");
        }

        auto symbol = to_symbol(car(args));
        if (!symbol) {
            throw EvalError("setq: " + std::to_string(car(args)) +
                            " is not a symbol.");
        }

        auto value = eval(cadr(args), env);
        set(env, symbol->name(), value);

        return value;
    }));

    set(env, "setq", make_proc([] (Cons args, EnvPtr env) {
        if (!args || !cdr(args)) {
            throw EvalError("setq: too few parameters");
        }
        if (cdr(cdr(args))) {
            throw EvalError("setq: too many parameters");
        }

        auto symbol = to_symbol(car(args));
        if (!symbol) {
            throw EvalError("setq: " + std::to_string(car(args)) +
                            " is not a symbol.");
        }

        auto value = eval(cadr(args), env);
        set(env, symbol->name(), value);

        return value;
    }));

    set(env, "do", make_proc([] (Cons args, EnvPtr env) {
        env = make_env(env);
        Object result;
        while (args) {
            result = eval(car(args), env);
            args = cdr(args);
        }
        return result;
    }));

    set(env, "+", make_proc([] (Cons args, EnvPtr env) {
        auto result = 0.0;
        for (; args; args = cdr(args)) {
            auto arg = eval(car(args), env);
            result += to_number(arg, "+").value();
        }
        return Number{ result };
    }));

    set(env, "-", make_proc([] (Cons args, EnvPtr env) {
        if (!args) {
            throw EvalError("-: too few parameters");
        }

        auto result = to_number(eval(car(args), env), "-").value();
        args = cdr(args);
        if (args) {
            while (args) {
                auto arg = eval(car(args), env);
                result -= to_number(arg, "-").value();
                args = cdr(args);
            }
        } else {
            // unary -
            result = -result;
        }

        return Number{ result };
    }));

    set(env, "*", make_proc([] (Cons args, EnvPtr env) {
        auto result = 1.0;
        for (; args; args = cdr(args)) {
            auto arg = eval(car(args), env);
            result *= to_number(arg, "*").value();
        }
        return Number{ result };
    }));

    set(env, "/", make_proc([] (Cons args, EnvPtr env) {
        if (!args || !cdr(args)) {
            throw EvalError("/: too few parameters");
        }

        auto result = to_number(eval(car(args), env), "/").value();
        for (args = cdr(args); args; args = cdr(args)) {
            auto arg = eval(car(args), env);
            result /= to_number(arg, "/").value();
        }
        return Number{ result };
    }));

    set(env, "nil?", make_proc([] (Cons args, EnvPtr env) {
        Object result;
        if (!eval(car(args), env)) {
            result = Symbol{ "t" };
        }
        return result;
    }));

    set(env, "zero?", make_proc([] (Cons args, EnvPtr env) {
        Object result;
        if (to_number(eval(car(args), env), "zero?").value() == 0) {
            result = Symbol{ "t" };
        }
        return result;
    }));

    set(env, "if", make_proc([] (Cons args, EnvPtr env) {
        auto cond = car(args);
        if (!cdr(args)) {
            throw EvalError("if: too few arguments");
        }

        auto body = cdr(args);
        auto then_arm = car(body);
        auto else_arm = cadr(body);

        if (eval(cond, env)) {
            return eval(then_arm, env);
        } else {
            return eval(else_arm, env);
        }
    }));

    set(env, "print", make_proc([] (Cons args, EnvPtr env) {
        auto first = true;
        while (args) {
            if (first) {
                first = false;
            } else {
                std::cout << " ";
            }
            std::cout << eval(car(args), env);
            args = cdr(args);
        }
        std::cout << std::endl;
        return Object{};
    }));

    set(env, "lambda", make_proc([] (Cons args, EnvPtr) {

        if (cdr(cdr(args))) {
            throw EvalError("lambda: too many args");
        }

        auto formal_args = to_formal_args(car(args), "lambda");
        auto lambda_body = cadr(args);

        return make_proc([formal_args, lambda_body] (Cons args, EnvPtr env) {

            auto lambda_env = make_env(env);

            auto syms = formal_args;
            while (syms) {
                assert(is_symbol(car(syms)));
                if (!args) {
                    EvalError("Proc: too few args");
                }

                auto sym = *to_symbol(car(syms));
                auto val = car(args);
                set(lambda_env, sym.name(), val);
                syms = cdr(syms);
                args = cdr(args);
            }

            if (args) {
                EvalError("Proc: too many args");
            }

            return eval(lambda_body, lambda_env);
        });
    }));

    set(env, "closure", make_proc([] (Cons args, EnvPtr env) {
        if (cdr(cdr(args))) {
            throw EvalError("closure: too many args");
        }

        auto formal_args = to_formal_args(car(args), "closure");
        auto closure_body = cadr(args);

        return make_proc([formal_args, closure_body, env] (Cons args, EnvPtr) {

            auto closure_env = make_env(env);

            auto syms = formal_args;
            while (syms) {
                assert(is_symbol(car(syms)));
                if (!args) {
                    EvalError("Proc: too few args");
                }

                auto sym = *to_symbol(car(syms));
                auto val = car(args);
                set(closure_env, sym.name(), val);
                syms = cdr(syms);
                args = cdr(args);
            }

            if (args) {
                EvalError("Proc: too many args");
            }

            return eval(closure_body, closure_env);
        });
    }));

    set(env, "def", make_proc([] (Cons args, EnvPtr env) {
        auto sym = to_symbol(car(args));
        if (!sym) {
            EvalError("def: " + std::to_string(car(args)) +
                      " is not a symbol.");
        }
        auto val = eval(make_cons(Symbol{ "lambda" }, cdr(args)), env);
        set(env, sym->name(), val);
        return val;
    }));

    return env;
}

int repl()
{
    auto parser = Parser{};
    auto env = build_env();

    auto readline = [] {
        std::string line;
        std::getline(std::cin, line);
        return line;
    };

    while (true) {
        if (parser.clean()) {
            std::cout << "mlisp> ";
        }
        else {
            std::cout << "...... ";
        }

        auto line = readline();
        auto is = std::istringstream(line);

        while (!is.eof()) {
            try {
                Object expr;
                if (!parser.parse(is, expr)) {
                    break;
                }
                std::cout << eval(expr, env) << std::endl;
            }
            catch (ParseError& e) {
                std::cout << e.what() << std::endl;
            }
            catch (EvalError& e) {
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

int run_file(const char* filename)
{
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        return -1;
    }

    try {
        auto parser = Parser{};
        auto env = build_env();

        Object expr;
        while (parser.parse(ifs, expr)) {
            eval(expr, env);
        }

        if (!ifs.eof()) {
            return -1;
        }
    }
    catch (ParseError& e) {
        std::cout << e.what() << std::endl;
        return -1;
    }
    catch (EvalError& e) {
        std::cout << e.what() << std::endl;
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    using namespace mlisp;

    if (argc == 1) {
        return repl();
    }

    return run_file(argv[1]);
}
