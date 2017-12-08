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

    auto quote = Proc{[] (List args, List env) {
        return args;
    }};
    auto env = cons(Symbol{"quote"}, cons(quote, {}));

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
                auto expr = parser.parse(is);
                if (!expr) {
                    break;
                }
                std::cout << expr << std::endl;
                //std::cout << eval(expr, env) << std::endl;
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
