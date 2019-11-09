#include "parser.hpp"

#include "number.hpp"

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
    set_custom_data_maker([](std::string const& token, bool quoted_string) -> std::shared_ptr<mll::Custom::Data> {
        if (!quoted_string) {
            double value;
            if (parse_number(token, &value)) {
                return std::make_shared<Number::Data>(value);
            }
        }
        return nullptr;
    });
}

} // namespace mlisp