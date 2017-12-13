#include <cassert>
#include <sstream>
#include <map>

#include "mlisp.hpp"

using namespace mlisp;

Node cadr(List list)
{
    return car(cdr(list));
}

std::shared_ptr<Env> build_env()
{
    auto car_proc = proc([] (List args, std::shared_ptr<Env> env) {
        if (cdr(args)) {
            throw EvalError("car: too many args given");
        }
        return car(eval(car(args), env).to_list());
    });

    auto cdr_proc = proc([] (List args, std::shared_ptr<Env> env) {
        if (cdr(args)) {
            throw EvalError("cdr: too many args given");
        }
        return cdr(eval(car(args), env).to_list());
    });

    auto cons_proc = proc([] (List args, std::shared_ptr<Env> env) {
        auto head = eval(car(args), env);
        if (!cdr(args)) {
            throw EvalError("cons: not enough args");
        }
        auto tail = eval(cadr(args), env).to_list();
        return cons(head, tail);
    });

    auto set_proc = proc([] (List args, std::shared_ptr<Env> env) {
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

    auto do_proc = proc([] (List args, std::shared_ptr<Env> env) {
        Node result;
        for (; args; args = cdr(args)) {
            result = eval(car(args), env);
        }
        return result;
    });

    auto plus_proc = proc([] (List args, std::shared_ptr<Env> env) {
        auto result = 0.0;
        for (; args; args = cdr(args)) {
            auto arg = eval(car(args), env);
            result += arg.to_number().value();
        }
        return number(result);
    });

    auto minus_proc = proc([] (List args, std::shared_ptr<Env> env) {
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

    auto mult_proc = proc([] (List args, std::shared_ptr<Env> env) {
        auto result = 1.0;
        for (; args; args = cdr(args)) {
            auto arg = eval(car(args), env);
            result *= arg.to_number().value();
        }
        return number(result);
    });

    auto devide_proc = proc([] (List args, std::shared_ptr<Env> env) {
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

    auto nilq_proc = proc([] (List args, std::shared_ptr<Env> env) {
        Node result;
        if (!eval(car(args), env)) {
            result = symbol("t");
        }
        return result;
    });

    auto zeroq_proc = proc([] (List args, std::shared_ptr<Env> env) {
        Node result;
        if (eval(car(args), env).to_number().value() == 0) {
            result = symbol("t");
        }
        return result;
    });

    auto if_proc = proc([] (List args, std::shared_ptr<Env> env) {
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

    auto print_proc = proc([] (List args, std::shared_ptr<Env> env) {
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

    auto lambda_proc = proc([] (List args, std::shared_ptr<Env> env) {
        auto largs = car(args).to_list();
        auto lbody = cadr(args);

        // validate largs (must be list of symbols)
        for (auto xxx = largs; xxx; xxx = cdr(xxx)) {
            auto arg = car(xxx);
            if (!arg.is_symbol()) {
                throw EvalError("lambda: " + std::to_string(arg) + " is not a symbol");
            }
        }

        if (cdr(cdr(args))) {
            throw EvalError("lambda: too many args");
        }

        return proc([largs, lbody] (List args, std::shared_ptr<Env> env) {

            env = make_env(env);

            auto syms = largs;
            while (syms) {
                assert(car(syms).is_symbol());
                if (!args) {
                    EvalError("Proc: too few args");
                }

                auto sym = car(syms).to_symbol();
                auto val = car(args);
                set(env, sym.name(), val);
                syms = cdr(syms);
                args = cdr(args);
            }

            if (args) {
                EvalError("Proc: too many args");
            }

            return eval(lbody, env);
        });
    });

    auto m = std::map<std::string, Node>{
        { "nil", {} },
        { "car", car_proc },
        { "cdr", cdr_proc },
        { "cons", cons_proc },
        { "set", set_proc },
        { "do", do_proc },
        { "+", plus_proc },
        { "-", minus_proc },
        { "*", mult_proc },
        { "/", devide_proc },
        { "nil?", nilq_proc },
        { "zero?", zeroq_proc },
        { "if", if_proc },
        { "print", print_proc },
        { "lambda", lambda_proc },
    };

    auto env = make_env(nullptr);
    for (auto const& pair: m) {
        set(env, pair.first, pair.second);
    }

    return env;
}

int main(int argc, char *argv[])
{
    using namespace mlisp;

    auto parser = Parser{};

    auto readline = [] {
        std::string line;
        std::getline(std::cin, line);
        return line;
    };

    auto evaluator = NodeEvaluator{ build_env() };

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
                std::cout << "expr: " << expr << std::endl;
                std::cout << "eval: " << evaluator.evaluate(expr) << std::endl;
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
