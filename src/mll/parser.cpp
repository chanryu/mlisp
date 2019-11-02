#include <mll/parser.hpp>

#include <mll/node.hpp>
#include <mll/symdef.hpp>

#include <array>
#include <cassert>
#include <sstream>

namespace mll {

namespace {
bool is_paren(char c)
{
    return c == '(' || c == ')';
}

bool is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

bool is_quote(char c)
{
    return c == '\'';
}

void skip_whitespaces_and_comments(std::istream& istream)
{
    auto in_comment = false;

    while (true) {
        auto c = istream.peek();
        if (c == EOF) {
            break;
        }

        if (in_comment) {
            istream.get();
            if (c == '\n') {
                in_comment = false;
            }
        }
        else if (c == ';') {
            istream.get();
            in_comment = true;
        }
        else if (is_space(c)) {
            istream.get();
        }
        else {
            break;
        }
    }
}

bool is_number_token(std::string const& token)
{
    assert(!token.empty());

    char const* s = token.c_str();
    if (*s == '-') s++;
    if (*s == '.') s++;
    if (*s < '0' || *s > '9') return false;

    size_t len;
    std::stod(token.c_str(), &len);
    return token.length() == len;
}

bool is_string_token(std::string const& token)
{
    assert(!token.empty());
    return token[0] == '"';
}

}

std::optional<Node>
Parser::parse(std::istream& istream)
{
    while (true) {

        if (!get_token(istream))
            break;

        assert(!token_.empty());

        if (token_ == "'") {
            token_.clear();
            stack_.push({ Context::Type::quote, {}, true });
            continue;
        }

        if (token_ == "(") {
            token_.clear();
            stack_.push({ Context::Type::paren, {}, true });
            continue;
        }

        Node node;

        if (token_ == ")") {
            List list;
            while (true) {
                if (stack_.empty() ||
                    stack_.top().type == Context::Type::quote) {
                    token_.clear();
                    throw ParseError{"redundant ')'"};
                }

                auto c = stack_.top();
                stack_.pop();
                if (c.head_empty) {
                    assert(c.type == Context::Type::paren);
                    assert(list.empty());
                }
                else {
                    list = cons(c.head, list);
                }
                if (c.type == Context::Type::paren) {
                    break;
                }
            }
            node = list;
        }
        else {
            node = make_node(std::move(token_));
        }

        token_.clear();

        while (true) {
            if (stack_.empty()) {
                return node;
            }

            if (stack_.top().type == Context::Type::quote) {
                stack_.pop();
                node = cons(Symbol{MLL_QUOTE}, cons(node, nil));
                continue;
            }

            if (stack_.top().head_empty) {
                stack_.top().head = node;
                stack_.top().head_empty = false;
            }
            else {
                stack_.push({ Context::Type::list, node, false });
            }
            break;
        }
    }

    return {};
}

bool
Parser::clean() const
{
    return stack_.empty();
}

Node
Parser::make_node(std::string token)
{
    if (is_number_token(token)) {
        return Number{std::stod(token)};
    }

    if (is_string_token(token)) {
        assert(token.length() >= 2);
        assert(token.front() == '"' && token.back() == '"');

        static const std::array<char, 128> esctbl = {{
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, '\"', 0x00, 0x00, 0x00, 0x00, '\'',
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\?',
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, '\\', 0x00, 0x00, 0x00,
            0x00, '\a', '\b', 0x00, 0x00, 0x00, '\f', 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\n', 0x00,
            0x00, 0x00, '\r', 0x00, '\t', 0x00, '\v', 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        }};

        std::string text;

        auto escaped = false;
        for (size_t i = 1; i < token.size() - 1; ++i) {
            auto c = token[i];
            if (escaped) {
                escaped = false;

                if (c >= 0 && static_cast<size_t>(c) < esctbl.size() && static_cast<bool>(esctbl[c])) {
                    text.push_back(esctbl[c]);
                }
                else {
                    text.push_back('\\');
                    text.push_back(c);
                }
            }
            else if (c == '\\') {
                escaped = true;
            }
            else {
                text.push_back(c);
            }
        }

        return String{std::move(text)};
    }

    return Symbol{std::move(token)};
}

bool
Parser::get_token(std::istream& istream)
{
    auto read_string_token = [this, &istream]() {
        assert(!token_.empty());
        assert(token_[0] == '"');
        char c;
        while (istream.get(c)) {
            token_.push_back(c);
            if (token_escaped_) {
                token_escaped_ = false;
            }
            else if (c == '\\') {
                token_escaped_ = true;
            }
            else if (c == '"') {
                return true;
            }
        }
        return false;
    };

    if (!token_.empty()) {
        // we have unfinished string token
        return read_string_token();
    }

    skip_whitespaces_and_comments(istream);

    char c;
    while (istream.get(c)) {
        if (is_space(c)) {
            assert(!token_.empty());
            return true;
        }

        if (is_quote(c)) {
            token_.push_back(c);
            return true;
        }

        if (is_paren(c)) {
            if (token_.empty()) {
                token_.push_back(c);
            }
            else {
                istream.unget();
            }
            return true;
        }

        if (c == '"') {
            if (token_.empty()) {
                token_.push_back(c);
                token_escaped_ = false;
                return read_string_token();
            }

            istream.unget();
            return true;
        }

        token_.push_back(c);
    }

    return !token_.empty();
}

} // namespace mll