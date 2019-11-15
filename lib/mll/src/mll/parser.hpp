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

class Parser {
public:
    std::optional<Node> parse(std::istream&); // throws ParseError
    bool clean() const;

    using CustomDataFunc =
        std::function<std::shared_ptr<Custom::Data>(std::string const& /*token*/, bool /*is_quoted*/)>;
    void set_custom_data_func(CustomDataFunc);

private:
    struct Context {
        std::string token;
        Node head;
        bool head_empty;
    };
    std::stack<Context> _stack;

    CustomDataFunc _custom_data_func;
};

} // namespace mll
