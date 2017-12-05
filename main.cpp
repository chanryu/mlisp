#include <sstream>

#include "mlisp.hpp"

class NodePrinter: mlisp::NodeVisitor {
public:
    explicit NodePrinter(std::ostream& ostream) : ostream_(ostream)
    {
    }

    void print(mlisp::Node const& node)
    {
        is_head_.push(true);
        node.accept(*this);
        is_head_.pop();
    }

private:
    void visit(mlisp::Symbol const& symbol) override
    {
        ostream_ << symbol.text;
    }

    void visit(mlisp::List const& list) override
    {
        if (!list.head) {
            ostream_ << "nil";
            return;
        }

        if (is_head_.top()) {
            ostream_ << '(';
        }

        is_head_.push(true);
        list.head->accept(*this);
        is_head_.pop();

        if (list.tail) {
            ostream_ << ' ';

            is_head_.push(false);
            list.tail->accept(*this);
            is_head_.pop();
        } else {
            ostream_ << ')';
        }
    }

private:
    std::stack<bool> is_head_;
    std::ostream& ostream_;
};

std::ostream&
operator << (std::ostream& os, mlisp::Node const& node)
{
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
    mlisp::Parser parser;

    while (true) {
        if (parser.clean()) {
            std::cout << "mlisp> ";
        } else {
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
                //std::cout << *mlisp::eval(expr) << std::endl;
                std::cout << *expr << std::endl;
            } catch (std::runtime_error& e) {
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
