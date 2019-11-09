#include "parser.hpp"

#include "number.hpp"
#include "string.hpp"

namespace {

bool parse_number(std::string const& text, double* value)
{
    assert(!text.empty());

    char const* s = text.c_str();
    if (*s == '-')
        s++;
    if (*s == '.')
        s++;
    if (*s < '0' || *s > '9')
        return false;

    size_t len;
    *value = std::stod(text.c_str(), &len);
    return text.length() == len;
}
} // namespace

namespace mlisp {

Parser::Parser()
{
    set_custom_data_func([](std::string const& token, bool is_quoted) {
        std::shared_ptr<mll::Custom::Data> data;
        if (is_quoted) {
            data = std::make_shared<String::Data>(token);
        }
        else if (double value; parse_number(token, &value)) {
            data = std::make_shared<Number::Data>(value);
        }
        return nullptr;
    });
}

} // namespace mlisp