#include "string.hpp"

#include "argc.hpp"
#include "bool.hpp"

#include <mll/env.hpp>
#include <mll/eval.hpp>
#include <mll/list.hpp>
#include <mll/print.hpp>
#include <mll/proc.hpp>

#define MLISP_DEFUN(cmd__, func__)                                                                                     \
    do {                                                                                                               \
        auto const cmd = cmd__;                                                                                        \
        env.set(cmd, Proc{cmd, func__});                                                                               \
    } while (0)

namespace mlisp {
namespace {
std::string quote_text(std::string const& text)
{
    std::string quoted_text;
    quoted_text.reserve(static_cast<size_t>(text.size() * 1.5) + 2);
    quoted_text.push_back('\"');
    for (auto c : text) {
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

bool is_string(mll::Node const& node)
{
    return mll::dynamic_node_cast<String>(node).has_value();
}

String to_string_or_throw(mll::Node const& node, char const* cmd)
{
    auto str = mll::dynamic_node_cast<String>(node);
    if (!str) {
        throw mll::EvalError(cmd + (": " + std::to_string(node)) + " is not a string.");
    }
    return *str;
}
} // namespace

void StringPrinter::print(std::ostream& ostream, mll::PrintContext context, std::string const& value)
{
    if (context == mll::PrintContext::inspect) {
        ostream << quote_text(value);
    }
    else {
        ostream << value;
    }
}

void set_string_procs(mll::Env& env)
{
    using namespace mll;

    MLISP_DEFUN("string?", [cmd](List const& args, Env& env) {
        assert_argc(args, 1, cmd);
        return to_node(is_string(eval(car(args), env)));
    });

    MLISP_DEFUN("string-equal?", [cmd](List const& args, Env& env) {
        assert_argc(args, 2, cmd);

        auto const str1 = to_string_or_throw(eval(car(args), env), cmd);
        auto const str2 = to_string_or_throw(eval(cadr(args), env), cmd);
        return to_node(str1.value() == str2.value());
    });
}

} // namespace mlisp