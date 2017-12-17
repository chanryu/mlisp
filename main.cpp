#include <cassert>
#include <fstream>
#include <sstream>

#include "mlisp.hpp"

using namespace mlisp;

bool is_symbol(Node node)
{
    return !!to_symbol(node);
}

List to_list(Node node, char const* cmd)
{
    auto list = to_list(node);
    if (!list) {
        throw EvalError(cmd + (": " + std::to_string(node)) + " is not a list.");
    }
    return *list;
}

Number to_number(Node node, char const* cmd)
{
    auto number = to_number(node);
    if (!number) {
        throw EvalError(cmd + (": " + std::to_string(node)) + " is not a number.");
    }
    return *number;
}

List to_formal_args(Node node, char const* cmd)
{
    auto args = to_list(node, cmd);

    // validate args (must be list of symbols)
    for (auto c = args; c; c = cdr(c)) {
        if (!is_symbol(car(c))) {
            throw EvalError(cmd + (": " + std::to_string(car(c))) + " is not a symbol");
        }
    }

    return args;
}

Node cadr(List list)
{
    return car(cdr(list));
}

Env build_env()
{
    auto env = Env{};

    env.set("car", make_proc([] (List args, Env env) {
        if (cdr(args)) {
            throw EvalError("car: too many args given");
        }
        return car(to_list(eval(car(args), env), "car"));
    }));

    env.set("cdr", make_proc([] (List args, Env env) {
        if (cdr(args)) {
            throw EvalError("cdr: too many args given");
        }
        return cdr(to_list(eval(car(args), env), "cdr"));
    }));

    env.set("cons", make_proc([] (List args, Env env) {
        if (!cdr(args)) {
            throw EvalError("cons: not enough args");
        }

        auto head = eval(car(args), env);
        auto tail = to_list(eval(cadr(args), env), "cons");

        return cons(head, tail);
    }));

    env.set("list", make_proc([] (List args, Env env) {
        std::vector<Node> objs;
        for (; args; args = cdr(args)) {
            objs.push_back(eval(car(args), env));
        }

        List list;
        while (!objs.empty()) {
            list = cons(objs.back(), list);
            objs.pop_back();
        }
        return list;
    }));

    env.set("set", make_proc([] (List args, Env env) {
        if (!args || !cdr(args)) {
            throw EvalError("set: too few parameters");
        }
        if (cdr(cdr(args))) {
            throw EvalError("set: too many parameters");
        }

        auto symbol = to_symbol(eval(car(args), env));
        if (!symbol) {
            throw EvalError("set: " + std::to_string(car(args)) +
                            " is not a symbol.");
        }

        auto value = eval(cadr(args), env);
        env.set(symbol->name(), value);

        return value;
    }));

    env.set("setq", make_proc([] (List args, Env env) {
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
        env.set(symbol->name(), value);

        return value;
    }));

    env.set("do", make_proc([] (List args, Env env) {
        env = env.derive_new();
        Node result;
        while (args) {
            result = eval(car(args), env);
            args = cdr(args);
        }
        return result;
    }));

    env.set("+", make_proc([] (List args, Env env) {
        auto result = 0.0;
        for (; args; args = cdr(args)) {
            auto arg = eval(car(args), env);
            result += to_number(arg, "+").value();
        }
        return make_number(result);
    }));

    env.set("-", make_proc([] (List args, Env env) {
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

        return make_number(result);
    }));

    env.set("*", make_proc([] (List args, Env env) {
        auto result = 1.0;
        for (; args; args = cdr(args)) {
            auto arg = eval(car(args), env);
            result *= to_number(arg, "*").value();
        }

        return make_number(result);
    }));

    env.set("/", make_proc([] (List args, Env env) {
        if (!args || !cdr(args)) {
            throw EvalError("/: too few parameters");
        }

        auto result = to_number(eval(car(args), env), "/").value();
        for (args = cdr(args); args; args = cdr(args)) {
            auto arg = eval(car(args), env);
            result /= to_number(arg, "/").value();
        }

        return make_number(result);
    }));

    env.set("nil?", make_proc([] (List args, Env env) {
        Node result;
        if (!eval(car(args), env)) {
            result = Symbol{ "t" };
        }
        return result;
    }));

    env.set("zero?", make_proc([] (List args, Env env) {
        Node result;
        if (to_number(eval(car(args), env), "zero?").value() == 0) {
            result = Symbol{ "t" };
        }
        return result;
    }));

    env.set("if", make_proc([] (List args, Env env) {
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

    env.set("print", make_proc([] (List args, Env env) {
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
    }));

    env.set("lambda", make_proc([] (List args, Env) {

        if (cdr(cdr(args))) {
            throw EvalError("lambda: too many args");
        }

        auto formal_args = to_formal_args(car(args), "lambda");
        auto lambda_body = cadr(args);

        return make_proc([formal_args, lambda_body] (List args, Env env) {

            auto lambda_env = env.derive_new();

            auto syms = formal_args;
            while (syms) {
                assert(is_symbol(car(syms)));
                if (!args) {
                    EvalError("Proc: too few args");
                }

                auto sym = *to_symbol(car(syms));
                auto val = eval(car(args), env);
                lambda_env.set(sym.name(), val);
                syms = cdr(syms);
                args = cdr(args);
            }

            if (args) {
                EvalError("Proc: too many args");
            }

            return eval(lambda_body, lambda_env);
        });
    }));

    env.set("closure", make_proc([] (List args, Env env) {
        if (cdr(cdr(args))) {
            throw EvalError("closure: too many args");
        }

        auto formal_args = to_formal_args(car(args), "closure");
        auto closure_body = cadr(args);
        auto closure_base_env = env;

        return make_proc([formal_args, closure_body, closure_base_env] (List args, Env env) {

            auto closure_env = closure_base_env.derive_new();

            auto syms = formal_args;
            while (syms) {
                assert(is_symbol(car(syms)));
                if (!args) {
                    EvalError("Proc: too few args");
                }

                auto sym = *to_symbol(car(syms));
                auto val = eval(car(args), env);
                closure_env.set(sym.name(), val);
                syms = cdr(syms);
                args = cdr(args);
            }

            if (args) {
                EvalError("Proc: too many args");
            }

            return eval(closure_body, closure_env);
        });
    }));

    env.set("def", make_proc([] (List args, Env env) {
        auto sym = to_symbol(car(args));
        if (!sym) {
            EvalError("def: " + std::to_string(car(args)) +
                      " is not a symbol.");
        }
        auto val = eval(cons(Symbol{ "lambda" }, cdr(args)), env);
        env.set(sym->name(), val);
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
                auto expr = parser.parse(is);
                if (!expr) {
                    break;
                }
                std::cout << eval(*expr, env) << std::endl;
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

        while (true) {
            auto expr = parser.parse(ifs);
            if (!expr) {
                break;
            }

            eval(*expr, env);
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
