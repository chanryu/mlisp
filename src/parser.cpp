#include "parser.hpp"

#include "fixnum.hpp"

namespace mlisp {

Parser::Parser()
{
    set_custom_data_maker([](std::string const& token, bool quoted_string) -> std::shared_ptr<mll::Custom::Data> {
        if (!quoted_string) {
            const char* b = token.c_str();
            char* e;
            auto value = std::strtol(b, &e, 10);
            if (static_cast<size_t>(e - b) == token.size()) {
                return std::make_shared<Fixnum::Data>(value);
            }
        }
        return nullptr;
    });
}

} // namespace mlisp