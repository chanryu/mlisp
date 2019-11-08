#pragma once

#include <mll/node.hpp>

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

private:
    struct Context {
        char token;
        Node head;
        bool head_empty;
    };
    std::stack<Context> stack_;
};

} // namespace mll
