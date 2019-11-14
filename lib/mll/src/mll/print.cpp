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
    explicit Printer(PrintContext context) : context_{context}
    {}

    void print(std::ostream& ostream, Node const& node)
    {
        ostream_ = &ostream;
        print(node, /* is_head */ true);
        ostream_ = nullptr;
    }

private:
    void visit(List const& list) override
    {
        assert(ostream_);

        if (list.empty()) {
            *ostream_ << "()";
            return;
        }

        const auto quote_token = get_quote_token(car(list));
        const auto quoted = quote_token != nullptr;

        if (quoted) {
            *ostream_ << quote_token;
        }
        else {
            if (is_head()) {
                *ostream_ << '(';
            }
            auto head = car(list);
            print(head, /* is_head */ true);
        }

        auto tail = cdr(list);
        if (!tail.empty()) {
            if (!quoted) {
                *ostream_ << ' ';
            }
            print(tail, /* is_head */ false);
        }

        if (!quoted && is_head()) {
            *ostream_ << ')';
        }
    }

    void visit(Proc const& proc) override
    {
        assert(ostream_);

        *ostream_ << "<#proc: " << proc.name() << ">";
    }

    void visit(Custom const& custom) override
    {
        assert(ostream_);

        custom.data()->print(*ostream_, context_);
    }

    void visit(Symbol const& sym) override
    {
        assert(ostream_);

        *ostream_ << sym.name();
    }

private:
    bool is_head() const
    {
        return is_head_stack_.top();
    }

    void print(Node const& node, bool is_head)
    {
        is_head_stack_.push(is_head);
        node.accept(*this);
        is_head_stack_.pop();
    }

    PrintContext const context_;
    std::ostream* ostream_ = nullptr;
    std::stack<bool, std::vector<bool>> is_head_stack_;
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