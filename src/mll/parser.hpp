#pragma once

#include <mll/custom.hpp>
#include <mll/node.hpp>

#include <functional>
#include <istream>
#include <optional>
#include <stack>
#include <stdexcept>

namespace mll {

class ParseError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

using CustomDataMaker =
    std::function<std::shared_ptr<Custom::Data>(std::string const& /*token*/, bool /*quoted_string*/)>;

class Parser {
public:
    std::optional<Node> parse(std::istream&); // throws ParseError
    bool clean() const;

    void set_custom_data_maker(CustomDataMaker);

private:
    struct Context {
        std::string token;
        Node head;
        bool head_empty;
    };
    std::stack<Context> stack_;

    CustomDataMaker custom_data_maker_;
};

} // namespace mll
