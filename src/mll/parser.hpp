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
    virtual ~Parser() = default;

    std::optional<Node> parse(std::istream&); // throws ParseError
    bool clean() const;

    virtual std::shared_ptr<Custom::Data> make_custom_data(std::string const& token, bool is_quoted) = 0;

private:
    struct Context {
        std::string token;
        Node head;
        bool head_empty;
    };
    std::stack<Context> stack_;
};

} // namespace mll
