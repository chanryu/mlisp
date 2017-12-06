#include <sstream>

#include "mlisp.hpp"

std::ostream&
operator << (std::ostream& os, mlisp::Node const& node)
{
    using namespace mlisp;

    class NodePrinter: NodeVisitor {
    public:
        explicit NodePrinter(std::ostream& ostream) : ostream_(ostream)
        {
        }

        void print(Node const& node)
        {
            node.accept(*this, true);
        }

    private:
        void visit(Symbol symbol, bool is_head) override
        {
            ostream_ << symbol.text();
        }

        void visit(List list, bool is_head) override
        {
            auto head = car(list);
            if (!head) {
                ostream_ << "nil";
                return;
            }

            if (is_head) {
                ostream_ << '(';
            }

            head.accept(*this, true);

            auto tail = cdr(list);
            if (tail) {
                ostream_ << ' ';

                tail.accept(*this, false);
            }
            else {
                ostream_ << ')';
            }
        }

    private:
        std::ostream& ostream_;
    };

    NodePrinter{os}.print(node);
    return os;
}

std::string
readline()
{
    std::string line;
    std::getline(std::cin, line);
    return line;
}

int main(int argc, char *argv[])
{
    mlisp::List env;
    mlisp::Parser parser;

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
                std::cout << mlisp::eval(expr, env) << std::endl;
            }
            catch (mlisp::ParseError& e) {
                std::cout << e.what() << std::endl;
            }
            catch (mlisp::EvalError& e) {
                std::cout << e.what() << std::endl;
            }
        }
        
        if (std::cin.eof()) {
            std::cout << std::endl;
            std::cout << "Bye." << std::endl;
            return parser.clean() ? 0 : -1;
        }
    }
}
