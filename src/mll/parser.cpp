#include <mll/parser.hpp>

#include <mll/node.hpp>
#include <mll/symdef.hpp>

#include <array>
#include <cassert>
#include <optional>
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

std::string read_text(std::istream& istream)
{
    std::string str;
    bool escaped = false;
    while (true) {
        char c;
        if (!istream.get(c)) {
            throw ParseError("malformed string: " + str);
        }
        if (escaped) {
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
            if (c >= 0 && static_cast<size_t>(c) < esctbl.size() && static_cast<bool>(esctbl[c])) {
                str.push_back(esctbl[c]);
            }
            else {
                str.push_back('\\');
                str.push_back(c);
            }
            escaped = false;
        }
        else if (c == '\\') {
            escaped = true;
        }
        else if (c == '"') {
            break;
        }
        else {
            str.push_back(c);
        }
    }
    return str;
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

struct Token {
    std::string text;
    bool is_string;
};

bool get_token(std::istream& istream, Token& token)
{
    token.text.clear();
    token.is_string = false;

    skip_whitespaces_and_comments(istream);

    char c;
    while (istream.get(c)) {
        if (is_space(c)) {
            assert(!token.text.empty());
            break;
        }

        if (is_paren(c)) {
            if (token.text.empty()) {
                token.text.push_back(c);
            }
            else {
                istream.unget();
            }
            return true;
        }

        if (c == '\'') {
            token.text.push_back(c);
            return true;
        }

        if (c == '"' && token.text.empty()) {
            token.text = read_text(istream);
            token.is_string = true;
            break;
        }

        token.text.push_back(c);
    }

    return !token.text.empty() || token.is_string;
}

bool parse_number(std::string const& text, double* value)
{
    assert(!text.empty());

    char const* s = text.c_str();
    if (*s == '-') s++;
    if (*s == '.') s++;
    if (*s < '0' || *s > '9') return false;

    size_t len;
    *value = std::stod(text.c_str(), &len);
    return text.length() == len;
}

}

std::optional<Node>
Parser::parse(std::istream& istream)
{
    Token token;
    while (get_token(istream, token)) {
        assert(!token.text.empty());

        Node node;

        if (token.is_string) {
            node = String{std::move(token.text)};
        }
        else if (token.text == "'") {
            stack_.push({ Context::Type::quote, {}, true });
            continue;
        }
        else if (token.text == "(") {
            stack_.push({ Context::Type::paren, {}, true });
            continue;
        }
        else if (token.text == ")") {
            List list;
            while (true) {
                if (stack_.empty() || stack_.top().type == Context::Type::quote) {
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
            if (double value; parse_number(token.text, &value)) {
                node = Number{value};
            }
            else {
                node = Symbol{std::move(token.text)};
            }
        }

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

} // namespace mll