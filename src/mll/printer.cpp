#include "printer.hpp"
#include "symdef.hpp"

namespace mll {

namespace {
std::string quote_text(std::string const& text)
{
    std::string quoted_text;
    quoted_text.reserve(static_cast<size_t>(text.size() * 1.5) + 2);
    quoted_text.push_back('\"');
    for (auto c: text) {
        if (c == '\"') {
            quoted_text.push_back('\\');
        }
        quoted_text.push_back(c);
    }
    quoted_text.push_back('\"');
    return quoted_text;
}
} // namespace

BasicPrinter::BasicPrinter(std::ostream& ostream)
    : ostream_(ostream)
{
}

void
BasicPrinter::print(Node const& node)
{
    print(node, /* is_head */ true);
}

void
BasicPrinter::print(Node const& node, bool is_head)
{
    is_head_stack_.push(is_head);
    node.accept(*this);
    is_head_stack_.pop();
}

void
BasicPrinter::visit(List const& list)
{
    if (list.empty()) {
        ostream_ << "()";
        return;
    }

    auto quoted = false;
    auto symbol = node_cast<Symbol>(car(list));

    if (symbol && symbol->name() == MLL_QUOTE) {
        ostream_ << "'";
        quoted = true;
    }

    if (!quoted && is_head()) {
        ostream_ << '(';
    }
    if (!quoted) {
        auto head = car(list);
        print(head, /* is_head */ true);
    }
    auto tail = cdr(list);
    if (!tail.empty()) {
        if (!quoted) {
            ostream_ << ' ';
        }
        print(tail, /* is_head */ false);
    }
    if (!quoted && is_head()) {
        ostream_ << ')';
    }
}

void
BasicPrinter::visit(Number const& num)
{
    ostream_ << num.value();
}

void
BasicPrinter::visit(String const& str)
{
    ostream_ << quote_text(str.text());
}

void
BasicPrinter::visit(Symbol const& sym)
{
    ostream_ << sym.name();
}

void
BasicPrinter::visit(Proc const& proc)
{
    ostream_ << "<#proc>";
}

bool
BasicPrinter::is_head() const
{
    return is_head_stack_.top();
}
} // namespace mll