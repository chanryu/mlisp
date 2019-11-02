#pragma once

#include <mll/node.hpp>

#include <iostream>
#include <optional>
#include <stack>
#include <stdexcept>
#include <string>

namespace mll {

class Node;

class Parser {
public:
    std::optional<Node> parse(std::istream&);
    bool clean() const;

protected:
    Node make_node(std::string token);

private:
    bool get_token(std::istream& istream);
    std::string token_;
    bool token_escaped_;

    struct Context {
        enum class Type { quote, paren, list };
        Type type;
        Node head;
        bool head_empty;
    };
    std::stack<Context> stack_;
};

struct ParseError: std::runtime_error {
    using runtime_error::runtime_error;
};

} // namespace mll
