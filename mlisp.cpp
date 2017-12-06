#include <cassert>

#include "mlisp.hpp"

namespace {
    using namespace mlisp;

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
}

///////////////////////////////////////////////////////////////////////////////
// Node::Data

struct mlisp::Node::Data: public std::enable_shared_from_this<Data> {
    virtual ~Data()
    {
    }

    virtual void accept(NodeVisitor&, bool) const = 0;
};


///////////////////////////////////////////////////////////////////////////////
// List::Data

struct mlisp::List::Data: public mlisp::Node::Data {

    Data(Node h, List t) noexcept : head{h}, tail{t}
    {
        assert(head || (!head && !tail));
    }

    void accept(NodeVisitor& visitor, bool is_head) const override
    {
        visitor.visit(List{ std::static_pointer_cast<Data const>(shared_from_this()) }, is_head);
    }
    
    Node const head;
    List const tail;
};

///////////////////////////////////////////////////////////////////////////////
// Symbol::Data

struct mlisp::Symbol::Data: public mlisp::Node::Data {
    
    explicit Data(std::string t) noexcept : text{std::move(t)}
    {
        assert(!text.empty());
    }

    void accept(NodeVisitor& visitor, bool is_head) const override
    {
        visitor.visit(Symbol{ std::static_pointer_cast<Data const>(shared_from_this()) }, is_head);
    }
    
    std::string const text;
};

///////////////////////////////////////////////////////////////////////////////
// Node

mlisp::Node::Node()
{
}

mlisp::Node::Node(Node const& other) : data_{other.data_}
{
}

mlisp::Node::Node(std::shared_ptr<Data const> data) : data_{data}
{
}

mlisp::Node&
mlisp::Node::operator = (Node const& rhs)
{
    data_ = rhs.data_;
    return *this;
}

mlisp::Node::operator bool() const
{
    return !!data_;
}

void
mlisp::Node::accept(NodeVisitor& visitor, bool is_head) const
{
    if (data_) {
        data_->accept(visitor, is_head);
    }
}

///////////////////////////////////////////////////////////////////////////////
// List

mlisp::List::List()
{
}

mlisp::List::List(List const& other) : Node{other.data_}
{
}

mlisp::List::List(std::shared_ptr<Data const> data) : Node{data}
{
}

mlisp::List&
mlisp::List::operator = (List const& rhs)
{
    data_ = rhs.data_;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////
// Symbol

mlisp::Symbol::Symbol()
{
}

mlisp::Symbol::Symbol(Symbol const& other) : Node{other.data_}
{
}

mlisp::Symbol::Symbol(std::shared_ptr<Data const> data) : Node{data}
{
}

mlisp::Symbol&
mlisp::Symbol::operator = (Symbol const& rhs)
{
    data_ = rhs.data_;
    return *this;
}

std::string const&
mlisp::Symbol::text() const
{
    assert(data_);
    return static_cast<Data const*>(data_.get())->text;
}

///////////////////////////////////////////////////////////////////////////////
// ParseError

mlisp::ParseError::ParseError(char const* what) : std::runtime_error{what}
{
}

mlisp::ParseError::ParseError(std::string const& what) : std::runtime_error{what}
{
}

///////////////////////////////////////////////////////////////////////////////
// Parser

Node
mlisp::Parser::parse(std::istream& istream)
{
    std::string token;

    while (true) {
        token = get_token(istream);

        if (token.empty()) {
            break;
        }

        if (token == "(") {
            stack_.push({ true, {} });
            continue;
        }

        Node expr;

        if (token == ")") {

            List list;
            while (true) {
                if (stack_.empty()) {
                    throw ParseError{"Unexpected ')'"};
                }

                auto c = stack_.top();
                stack_.pop();
                list = cons(c.head, list);
                if (c.paren) {
                    break;
                }
            }
            expr = list;
        } else {
            expr = intern(token);
        }
            
        if (stack_.empty()) {
            return expr;
        } else {
            if (stack_.top().head) {
                stack_.push({ false, expr });
            } else {
                stack_.top().head = expr;
            }
        }
    }

    return {};
}

bool
mlisp::Parser::clean() const noexcept
{
    return stack_.empty();
}

mlisp::Symbol
mlisp::Parser::intern(std::string text) noexcept
{
    auto i = symbols_.find(text);
    if (i != symbols_.end()) {
        return i->second;
    }

    Symbol symbol{ std::make_shared<Symbol::Data>(text) };
    symbols_[text] = symbol;
    return symbol;
}

///////////////////////////////////////////////////////////////////////////////
// EvalError

mlisp::EvalError::EvalError(char const* what) : std::runtime_error{what}
{
}

mlisp::EvalError::EvalError(std::string const& what) : std::runtime_error{what}
{
}

///////////////////////////////////////////////////////////////////////////////
// eval

mlisp::Node
mlisp::eval(Node expr, List env)
{
    return expr;
}

///////////////////////////////////////////////////////////////////////////////
// Builtins

mlisp::Node
mlisp::car(List list) noexcept
{
    assert(list.data_);
    return static_cast<List::Data const*>(list.data_.get())->head;
}

mlisp::List
mlisp::cdr(List list) noexcept
{
    assert(list.data_);
    return static_cast<List::Data const*>(list.data_.get())->tail;
}

mlisp::List
mlisp::cons(Node head, List tail) noexcept
{
    if (!head) {
        static List nil{ std::make_shared<List::Data>(Node{}, List{}) };
        return nil;
    }

    return List{ std::make_shared<List::Data>(head, tail) };
}
