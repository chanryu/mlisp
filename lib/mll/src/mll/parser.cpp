#include <mll/parser.hpp>

#include <mll/custom.hpp>
#include <mll/list.hpp>
#include <mll/quote.hpp>
#include <mll/symbol.hpp>

#include <array>
#include <cassert>
#include <optional>
#include <sstream>

namespace {

constexpr std::array<char, 128> esctbl = {{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\"', 0x00, 0x00, 0x00,
    0x00, '\'', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\?', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\\', 0x00, 0x00,
    0x00, 0x00, '\a', '\b', 0x00, 0x00, 0x00, '\f', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\n', 0x00, 0x00, 0x00,
    '\r', 0x00, '\t', 0x00, '\v', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
}};

bool is_whitespace(char c)
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
            throw mll::ParseError("malformed string: " + str);
        }
        if (escaped) {
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
        else if (is_whitespace(c)) {
            istream.get();
        }
        else {
            break;
        }
    }
}

struct Token {
    std::string text;
    bool is_double_quoted;
};

bool get_token(std::istream& istream, Token& token)
{
    token.text.clear();
    token.is_double_quoted = false;

    skip_whitespaces_and_comments(istream);

    char c;
    while (istream.get(c)) {
        if (is_whitespace(c)) {
            assert(!token.text.empty());
            break;
        }

        if (c == '(' || c == ')') {
            if (token.text.empty()) {
                token.text.push_back(c);
            }
            else {
                istream.unget();
            }
            return true;
        }

        if (c == '\'' || c == '`' || c == ',') {
            token.text.push_back(c);
            if (c == ',' && istream.get(c)) {
                if (c == '@') {
                    token.text.push_back(c);
                }
                else {
                    istream.unget();
                }
            }
            return true;
        }

        if (c == '"' && token.text.empty()) {
            token.text = read_text(istream);
            token.is_double_quoted = true;
            break;
        }

        token.text.push_back(c);
    }

    return !token.text.empty() || token.is_double_quoted;
}

} // namespace

namespace mll {
std::optional<Node> Parser::parse(std::istream& istream)
{
    auto make_custom_or_symbol = [this](Token const& token) -> Node {
        std::shared_ptr<Custom::Core> custom_data;
        if (_custom_data_func) {
            custom_data = _custom_data_func(token.text, token.is_double_quoted);
        }
        if (custom_data) {
            return Custom{custom_data};
        }
        return Symbol{token.text};
    };

    Token token;
    while (get_token(istream, token)) {
        assert(!token.text.empty());

        Node node;

        if (token.is_double_quoted) {
            node = make_custom_or_symbol(token);
        }
        else if (token.text == "(" || is_quote_token(token.text)) {
            _stack.push({token.text, {}, true});
            continue;
        }
        else if (token.text == ")") {
            List list;
            while (true) {
                if (_stack.empty() || quote_symbol_name_from_token(_stack.top().token) != nullptr) {
                    throw mll::ParseError{"redundant ')'"};
                }

                auto c = _stack.top();
                _stack.pop();
                if (c.head_empty) {
                    assert(c.token == "(");
                    assert(list.empty());
                }
                else {
                    list = cons(c.head, list);
                }
                if (c.token == "(") {
                    break;
                }
            }
            node = list;
        }
        else {
            node = make_custom_or_symbol(token);
        }

        while (true) {
            if (_stack.empty()) {
                return node;
            }

            if (auto quote_name = quote_symbol_name_from_token(_stack.top().token)) {
                _stack.pop();
                node = cons(Symbol{quote_name}, cons(node, nil));
                continue;
            }

            if (_stack.top().head_empty) {
                _stack.top().head = node;
                _stack.top().head_empty = false;
            }
            else {
                _stack.push({/*token*/ "", node, false});
            }
            break;
        }
    }

    return {};
}

bool Parser::clean() const
{
    return _stack.empty();
}

void Parser::set_custom_data_func(CustomDataFunc custom_data_func)
{
    _custom_data_func = std::move(custom_data_func);
}

} // namespace mll