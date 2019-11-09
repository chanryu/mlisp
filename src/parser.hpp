#pragma once

#include <mll/parser.hpp>

namespace mlisp {

class Parser : public mll::Parser {
public:
    std::shared_ptr<mll::Custom::Data> make_custom_data(std::string const& token, bool is_quoted) override;
};

} // namespace mlisp