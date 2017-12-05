#include <cassert>

#include "mlisp.hpp"

namespace {
    using namespace mlisp;

    std::shared_ptr<Node> car(std::shared_ptr<List> list) noexcept
    {
        return list->head;
    }

    std::shared_ptr<List> cdr(std::shared_ptr<List> list) noexcept
    {
        return list->tail;
    }

    std::shared_ptr<List> nil() noexcept
    {
        static auto nil = std::make_shared<List>(nullptr, nullptr);
        return nil;
    }

    std::shared_ptr<List> cons(std::shared_ptr<Node> head, std::shared_ptr<List> tail) noexcept
    {
        if (!head) {
            assert(!tail);
            return nil();
        }

        return std::make_shared<List>(head, tail);
    }

    bool is_paren(int c) noexcept
    {
        return c == '(' || c == ')';
    }

    bool is_space(int c) noexcept
    {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }

    void skip_spaces(std::istream& istream) noexcept
    {
        while (is_space(istream.peek())) {
            istream.get();
        }
    }

    std::string get_token(std::istream& istream) noexcept
    {
        skip_spaces(istream);

        std::string token;
        while (istream.good()) {
            auto c = istream.get();
            if (is_space(c)) {
                assert(!token.empty());
                break;
            }
            if (is_paren(c)) {
                if (token.empty()) {
                    token.push_back(c);
                } else {
                    istream.unget();
                }
                break;
            }
            token.push_back(c);
        }

        return token;
    }

    using EvalError = std::runtime_error;
    using ParseError = std::runtime_error;
}

///////////////////////////////////////////////////////////////////////////////
// Node

mlisp::Node::~Node()
{
}

///////////////////////////////////////////////////////////////////////////////
// Symbol

mlisp::Symbol::Symbol(std::string t) noexcept : text{std::move(t)}
{
    assert(!text.empty());
}

void
mlisp::Symbol::accept(NodeVisitor& visitor) const
{
    visitor.visit(*this);
}

///////////////////////////////////////////////////////////////////////////////
// List

mlisp::List::List(std::shared_ptr<Node> h, std::shared_ptr<List> t) noexcept : head{h}, tail{t}
{
    assert(head || (!head && !tail));
}

void
mlisp::List::accept(NodeVisitor& visitor) const
{
    visitor.visit(*this);
}

///////////////////////////////////////////////////////////////////////////////
// Parser

std::shared_ptr<Node>
mlisp::Parser::parse(std::istream& istream)
{
    std::string token;

    while (true) {
        token = get_token(istream);

        if (token.empty()) {
            break;
        }

        if (token == "(") {
            stack_.push({ true, nullptr });
            continue;
        }

        std::shared_ptr<Node> node;

        if (token == ")") {

            std::shared_ptr<List> list;
            while (true) {
                if (stack_.empty()) {
                    throw ParseError{"Unexpected ')'"};
                }

                auto c = stack_.top();
                stack_.pop();
                list = cons(c.node, list);
                if (c.paran) {
                    break;
                }
            }
            node = list;
        } else {
            node = intern(token);
        }
            
        if (stack_.empty()) {
            return node;
        } else {
            if (stack_.top().node) {
                stack_.push({ false, node });
            } else {
                stack_.top().node = node;
            }
        }
    }

    return nullptr;
}

bool
mlisp::Parser::clean() const noexcept
{
    return stack_.empty();
}

std::shared_ptr<Symbol>
mlisp::Parser::intern(std::string text) noexcept
{
    auto i = symbols_.find(text);
    if (i != symbols_.end()) {
        return i->second;
    }
    auto symbol = std::make_shared<Symbol>(text);
    symbols_[text] = symbol;
    return symbol;
}

