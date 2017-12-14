#include <cassert>
#include <fstream>
#include <sstream>

#include "mlisp.hpp"

using namespace mlisp;

Node cadr(Cons list)
{
    return car(cdr(list));
}

EnvPtr build_env()
{
    auto car_proc = procedure([] (Cons args, EnvPtr env) {
        if (cdr(args)) {
            throw EvalError("car: too many args given");
        }
        return car(eval(car(args), env).to_cons());
    });

    auto cdr_proc = procedure([] (Cons args, EnvPtr env) {
        if (cdr(args)) {
            throw EvalError("cdr: too many args given");
        }
        return cdr(eval(car(args), env).to_cons());
    });

    auto cons_proc = procedure([] (Cons args, EnvPtr env) {
        auto head = eval(car(args), env);
        if (!cdr(args)) {
            throw EvalError("cons: not enough args");
        }
        auto tail = eval(cadr(args), env).to_cons();
        return cons(head, tail);
    });

    auto list_proc = procedure([] (Cons args, EnvPtr env) {
        std::vector<Node> objs;
        for (; args; args = cdr(args)) {
            objs.push_back(eval(car(args), env));
        }

        Cons list;
        while (!objs.empty()) {
            list = cons(objs.back(), list);
            objs.pop_back();
        }
        return list;
    });

    auto setq_proc = procedure([] (Cons args, EnvPtr env) {
        if (!args || !cdr(args)) {
            throw EvalError("setq: too few parameters");
        }
        if (cdr(cdr(args))) {
            throw EvalError("setq: too many parameters");
        }

        auto name = car(args).to_symbol().name();
        auto value = eval(cadr(args), env);
        set(env, name, value);
        return value;
    });

    auto set_proc = procedure([] (Cons args, EnvPtr env) {
        if (!args || !cdr(args)) {
            throw EvalError("set: too few parameters");
        }
        if (cdr(cdr(args))) {
            throw EvalError("set: too many parameters");
        }

        auto name = eval(car(args), env).to_symbol().name();
        auto value = eval(cadr(args), env);
        set(env, name, value);
        return value;
    });

    auto do_proc = procedure([] (Cons args, EnvPtr env) {
        env = make_env(env);
        Node result;
        while (args) {
            result = eval(car(args), env);
            args = cdr(args);
        }
        return result;
    });

    auto plus_proc = procedure([] (Cons args, EnvPtr env) {
        auto result = 0.0;
        for (; args; args = cdr(args)) {
            auto arg = eval(car(args), env);
            result += arg.to_number().value();
        }
        return number(result);
    });

    auto minus_proc = procedure([] (Cons args, EnvPtr env) {
        if (!args) {
            throw EvalError("-: too few parameters");
        }

        auto result = eval(car(args), env).to_number().value();
        args = cdr(args);
        if (args) {
            while (args) {
                auto arg = eval(car(args), env);
                result -= arg.to_number().value();
                args = cdr(args);
            }
        } else {
            // unary -
            result = -result;
        }

        return number(result);
    });

    auto mult_proc = procedure([] (Cons args, EnvPtr env) {
        auto result = 1.0;
        for (; args; args = cdr(args)) {
            auto arg = eval(car(args), env);
            result *= arg.to_number().value();
        }
        return number(result);
    });

    auto devide_proc = procedure([] (Cons args, EnvPtr env) {
        if (!args || !cdr(args)) {
            throw EvalError("/: too few parameters");
        }

        auto result = eval(car(args), env).to_number().value();
        for (args = cdr(args); args; args = cdr(args)) {
            auto arg = eval(car(args), env);
            result /= arg.to_number().value();
        }
        return number(result);
    });

    auto nilq_proc = procedure([] (Cons args, EnvPtr env) {
        Node result;
        if (!eval(car(args), env)) {
            result = symbol("t");
        }
        return result;
    });

    auto zeroq_proc = procedure([] (Cons args, EnvPtr env) {
        Node result;
        if (eval(car(args), env).to_number().value() == 0) {
            result = symbol("t");
        }
        return result;
    });

    auto if_proc = procedure([] (Cons args, EnvPtr env) {
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
    });

    auto print_proc = procedure([] (Cons args, EnvPtr env) {
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
        return Node{};
    });

    auto lambda_proc = procedure([] (Cons args, EnvPtr) {
        auto formal_args = car(args).to_cons();
        auto lambda_body = cadr(args);

        // validate formal_args (must be list of symbols)
        for (auto xxx = formal_args; xxx; xxx = cdr(xxx)) {
            auto arg = car(xxx);
            if (!arg.is_symbol()) {
                throw EvalError("lambda: " + std::to_string(arg) + " is not a symbol");
            }
        }

        if (cdr(cdr(args))) {
            throw EvalError("lambda: too many args");
        }

        return procedure([formal_args, lambda_body] (Cons args, EnvPtr env) {

            auto lambda_env = make_env(env);

            auto syms = formal_args;
            while (syms) {
                assert(car(syms).is_symbol());
                if (!args) {
                    EvalError("Procedure: too few args");
                }

                auto sym = car(syms).to_symbol();
                auto val = car(args);
                set(lambda_env, sym.name(), val);
                syms = cdr(syms);
                args = cdr(args);
            }

            if (args) {
                EvalError("Procedure: too many args");
            }

            return eval(lambda_body, lambda_env);
        });
    });

    auto closure_proc = procedure([] (Cons args, EnvPtr env) {
        auto formal_args = car(args).to_cons();
        auto closure_body = cadr(args);

        // validate formal_args (must be list of symbols)
        for (auto xxx = formal_args; xxx; xxx = cdr(xxx)) {
            auto arg = car(xxx);
            if (!arg.is_symbol()) {
                throw EvalError("closure: " + std::to_string(arg) + " is not a symbol");
            }
        }

        if (cdr(cdr(args))) {
            throw EvalError("closure: too many args");
        }

        return procedure([formal_args, closure_body, env] (Cons args, EnvPtr) {

            auto closure_env = make_env(env);

            auto syms = formal_args;
            while (syms) {
                assert(car(syms).is_symbol());
                if (!args) {
                    EvalError("Procedure: too few args");
                }

                auto sym = car(syms).to_symbol();
                auto val = car(args);
                set(closure_env, sym.name(), val);
                syms = cdr(syms);
                args = cdr(args);
            }

            if (args) {
                EvalError("Procedure: too many args");
            }

            return eval(closure_body, closure_env);
        });
    });

    auto def_proc = procedure([] (Cons args, EnvPtr env) {
        auto sym = car(args).to_symbol();
        auto val = eval(cons(symbol("lambda"), cdr(args)), env);
        assert(val.is_procedure());
        set(env, sym.name(), val);
        return val;
    });


    auto env = make_env(nullptr);

    set(env, "car", car_proc);
    set(env, "cdr", cdr_proc);
    set(env, "cons", cons_proc);
    set(env, "list", list_proc);
    set(env, "set", set_proc);
    set(env, "setq", setq_proc);
    set(env, "do", do_proc);

    set(env, "+", plus_proc);
    set(env, "-", minus_proc);
    set(env, "*", mult_proc);
    set(env, "/", devide_proc);

    set(env, "nil?", nilq_proc);
    set(env, "zero?", zeroq_proc);

    set(env, "if", if_proc);
    set(env, "print", print_proc);

    set(env, "lambda", lambda_proc);
    set(env, "closure", closure_proc);
    set(env, "def", def_proc);

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
                Node expr;
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

        Node expr;
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
