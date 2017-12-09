#include <sstream>

#include "mlisp.hpp"

std::string
readline()
{
    std::string line;
    std::getline(std::cin, line);
    return line;
}

int main(int argc, char *argv[])
{
    using namespace mlisp;

    auto quote_proc = Proc{[] (List args, List env) {
        return args;
    }};
    auto car_proc = Proc{[] (List args, List env) {
        if (cdr(args)) {
            throw EvalError("car: too many args given");
        }
        return car(eval(car(args), env).to_list());
    }};
    auto cdr_proc = Proc{[] (List args, List env) {
        if (cdr(args)) {
            throw EvalError("cdr: too many args given");
        }
        return cdr(eval(car(args), env).to_list());
    }};

    List env;
    env = cons(Symbol{"nil"}, cons({}, env));
    env = cons(Symbol{"car"}, cons(car_proc, env));
    env = cons(Symbol{"cdr"}, cons(cdr_proc, env));
    env = cons(Symbol{"quote"}, cons(quote_proc, env));

    Parser parser;

    while (true) {
        if (parser.clean()) {
            std::cout << "mlisp> ";
        }
        else {
            std::cout << "...... ";
        }

        std::string line = readline();
        std::istringstream is(line);

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
