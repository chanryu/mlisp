#include <sstream>

#include "mlisp.hpp"

mlisp::List build_env()
{
    using namespace mlisp;

    auto car_proc = proc([] (List args, List env) {
        if (cdr(args)) {
            throw EvalError("car: too many args given");
        }
        return car(eval(car(args), env).to_list());
    });

    auto cdr_proc = proc([] (List args, List env) {
        if (cdr(args)) {
            throw EvalError("cdr: too many args given");
        }
        return cdr(eval(car(args), env).to_list());
    });

    auto cons_proc = proc([] (List args, List env) {
        auto head = eval(car(args), env);
        if (!cdr(args)) {
            throw EvalError("cons: not enough args");
        }
        auto tail = eval(car(cdr(args)), env).to_list();
        return cons(head, tail);
    });

    auto eval_proc = proc([] (List args, List env) {
        Node result;
        while (args) {
            result = eval(eval(car(args), env), env);
            args = cdr(args);
        }
        return result;
    });

    auto plus_proc = proc([] (List args, List env) {
        auto result = 0.0;
        while (args) {
            auto arg = eval(car(args), env);
            result += arg.to_number().value();
            args = cdr(args);
        }
        return number(result);
    });

    auto m = std::map<std::string, mlisp::Node>{
        { "nil", {} },
        { "car", car_proc },
        { "cdr", cdr_proc },
        { "cons", cons_proc },
        { "eval", eval_proc },
        { "+", plus_proc },
    };

    List env;
    for (auto const& pair: m) {
        env = cons(symbol(pair.first), cons(pair.second, env));
    }

    return env;
}

int main(int argc, char *argv[])
{
    using namespace mlisp;

    auto env = build_env();
    auto parser = Parser{};

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
                std::cout << "expr: " << expr << std::endl;
                std::cout << "eval: " << eval(expr, env) << std::endl;
            }
            catch (mlisp::ParseError& e) {
                std::cout << e.what() << std::endl;
            }
            catch (mlisp::EvalError& e) {
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
