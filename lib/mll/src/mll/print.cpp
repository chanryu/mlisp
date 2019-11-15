#include <mll/print.hpp>

#include <mll/custom.hpp>
#include <mll/node.hpp>
#include <mll/quote.hpp>

#include <cassert>
#include <sstream>
#include <stack>
#include <vector>

namespace mll {

namespace {

const char* get_quote_token(Node const& node)
{
    if (auto sym = dynamic_node_cast<Symbol>(node)) {
        return quote_token_from_symbol_name(sym->name());
    }
    return nullptr;
}

class Printer : NodeVisitor {
public:
    explicit Printer(PrintContext context) : _context{context}
    {}

    void print(std::ostream& ostream, Node const& node)
    {
        _ostream = &ostream;
        print(node, /* is_head */ true);
        _ostream = nullptr;
    }

private:
    void visit(List const& list) override
    {
        assert(_ostream);

        if (list.empty()) {
            *_ostream << "()";
            return;
        }

        const auto quote_token = get_quote_token(car(list));
        const auto quoted = quote_token != nullptr;

        if (quoted) {
            *_ostream << quote_token;
        }
        else {
            if (is_head()) {
                *_ostream << '(';
            }
            auto head = car(list);
            print(head, /* is_head */ true);
        }

        auto tail = cdr(list);
        if (!tail.empty()) {
            if (!quoted) {
                *_ostream << ' ';
            }
            print(tail, /* is_head */ false);
        }

        if (!quoted && is_head()) {
            *_ostream << ')';
        }
    }

    void visit(Proc const& proc) override
    {
        assert(_ostream);

        *_ostream << "<#proc: " << proc.name() << ">";
    }

    void visit(Custom const& custom) override
    {
        assert(_ostream);

        custom.data()->print(*_ostream, _context);
    }

    void visit(Symbol const& sym) override
    {
        assert(_ostream);

        *_ostream << sym.name();
    }

private:
    bool is_head() const
    {
        return _is_head_stack.top();
    }

    void print(Node const& node, bool is_head)
    {
        _is_head_stack.push(is_head);
        node.accept(*this);
        _is_head_stack.pop();
    }

    PrintContext const _context;
    std::ostream* _ostream = nullptr;
    std::stack<bool, std::vector<bool>> _is_head_stack;
};
} // namespace

void print(std::ostream& ostream, Node const& node, PrintContext context)
{
    Printer{context}.print(ostream, node);
}
} // namespace mll

namespace std {
string to_string(mll::Node const& node)
{
    ostringstream ss;
    print(ss, node);
    return ss.str();
}
} // namespace std