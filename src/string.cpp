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

///////////////////////////////////////////////////////////////////////////////
// String::Data

String::Data::Data(std::string v) : value{std::move(v)}
{}

void String::Data::print(std::ostream& ostream, mll::PrintOptions const& options)
{
    if (options.quote_string) {
        ostream << quote_text(value);
    }
    else {
        ostream << value;
    }
}

///////////////////////////////////////////////////////////////////////////////
// String

String::String(std::string value) : mll::Custom{std::make_shared<Data>(std::move(value))}
{}

String::String(String const& other) : mll::Custom{other}
{}

String::String(std::shared_ptr<Data> const& data) : mll::Custom{data}
{}

std::string const& String::value() const
{
    return static_cast<Data*>(Custom::data().get())->value;
}

std::optional<String> String::from_node(mll::Node const& node)
{
    if (auto data = std::dynamic_pointer_cast<Data>(node.data())) {
        return String{data};
    }
    return std::nullopt;
}

} // namespace mlisp