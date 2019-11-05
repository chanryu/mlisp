#include <mll/print.hpp>

#include <mll/node.hpp>
#include <mll/symdef.hpp>

#include <cassert>
#include <cmath>
#include <iomanip>
#include <stack>
#include <vector>

namespace mll {

namespace {
std::string quote_text(std::string const& text)
{
    std::string quoted_text;
    quoted_text.reserve(static_cast<size_t>(text.size() * 1.5) + 2);
    quoted_text.push_back('\"');
    for (auto c: text) {
        switch (c) {
        case '\"':
            quoted_text.append("\\\"");
            break;
        case '\a':
            quoted_text.append("\\a");
            break;
        case '\b':
            quoted_text.append("\\b");
            break;
        case '\r':
            quoted_text.append("\\r");
            break;
        case '\n':
            quoted_text.append("\\n");
            break;
        default:
            quoted_text.push_back(c);
            break;
        }
    }
    quoted_text.push_back('\"');
    return quoted_text;
}

class Printer: NodeVisitor {
public:
    explicit Printer(StringStyle string_style)
        : string_style_{string_style} {}

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

        auto quoted = false;
        auto symbol = dynamic_node_cast<Symbol>(car(list));

        if (symbol && symbol->name() == MLL_QUOTE) {
            *ostream_ << "'";
            quoted = true;
        }

        if (!quoted && is_head()) {
            *ostream_ << '(';
        }
        if (!quoted) {
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

    void visit(Number const& num) override
    {
        assert(ostream_);

        const bool is_integral = [&num] {
            decltype(num.value()) int_part;
            std::modf(num.value(), &int_part);
            return num.value() == int_part;
        }();
        if (is_integral) {
            *ostream_ << std::setprecision(0);
        }

        *ostream_ << std::fixed << num.value();
    }

    void visit(String const& str) override
    {
        assert(ostream_);

        if (string_style_ == StringStyle::quoted) {
            *ostream_ << quote_text(str.text());
        }
        else {
            *ostream_ << str.text();
        }
    }

    void visit(Symbol const& sym) override
    {
        assert(ostream_);

        *ostream_ << sym.name();
    }

    void visit(Proc const& proc) override
    {
        assert(ostream_);

        *ostream_ << "<#proc>";
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

    const StringStyle string_style_;
    std::ostream* ostream_ = nullptr;
    std::stack<bool, std::vector<bool>> is_head_stack_;
};
} // namespace

void print(std::ostream& ostream, Node const& node, StringStyle string_style)
{
    Printer{string_style}.print(ostream, node);
}
} // namespace mll