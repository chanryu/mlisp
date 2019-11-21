#include "string.hpp"

#include <mll/print.hpp>

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
} // namespace

namespace mlisp {
MLL_CUSTOM_TYPE_IMPL(String, std::string, [](auto& ostream, auto context, auto const& value) {
    if (context == mll::PrintContext::inspect) {
        ostream << quote_text(value);
    }
    else {
        ostream << value;
    }
});
} // namespace mlisp