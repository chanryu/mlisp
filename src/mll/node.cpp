#include <mll/node.hpp>

#include <mll/symdef.hpp>

#include <array>
#include <cassert>
#include <map>

namespace mll {

List const nil;


////////////////////////////////////////////////////////////////////////////////
// Node::Data

struct Node::Data: std::enable_shared_from_this<Node::Data>
{
    virtual ~Data() = default;
    virtual void accept(NodeVisitor&) = 0;
};

////////////////////////////////////////////////////////////////////////////////
// List::Data

struct List::Data: Node::Data
{
    Data(Node const& h, List const& t) : head{h}, tail{t} { }

    void accept(NodeVisitor& visitor) override
    {
        visitor.visit(List{
            std::static_pointer_cast<Data>(shared_from_this())
        });
    }

    Node const head;
    List const tail;
};

////////////////////////////////////////////////////////////////////////////////
// Proc::Data

struct Proc::Data: Node::Data
{
    explicit Data(Func f) : func{std::move(f)} { }

    void accept(NodeVisitor& visitor) override
    {
        visitor.visit(Proc{
            std::static_pointer_cast<Data>(shared_from_this())
        });
    }

    Func const func;
};

////////////////////////////////////////////////////////////////////////////////
// Number::Data

struct Number::Data: Node::Data
{
    explicit Data(double v) : value{v} { }

    void accept(NodeVisitor& visitor) override
    {
        visitor.visit(Number{
            std::static_pointer_cast<Data>(shared_from_this())
        });
    }

    double const value;
};

////////////////////////////////////////////////////////////////////////////////
// String::Data

struct String::Data: Node::Data
{
    explicit Data(std::string t) : text{std::move(t)} { }

    void accept(NodeVisitor& visitor) override
    {
        visitor.visit(String{
            std::static_pointer_cast<Data>(shared_from_this())
        });
    }

    std::string const text;
};

////////////////////////////////////////////////////////////////////////////////
// Symbol::Data

struct Symbol::Data: Node::Data
{
    explicit Data(std::string n) : name{std::move(n)} { }

    void accept(NodeVisitor& visitor) override
    {
        visitor.visit(Symbol{
            std::static_pointer_cast<Data>(shared_from_this())
        });
    }

    std::string const name;
};

////////////////////////////////////////////////////////////////////////////////
// Object

Node::Node(Node const& other)
    : data_{other.data_}
{
}

Node::Node(List const& list)
    : data_{list.data_}
{
}

Node::Node(Proc const& proc)
    : data_{proc.data_}
{
}

Node::Node(Number const& number)
    : data_{number.data_}
{
}

Node::Node(String const& string)
    : data_{string.data_}
{
}

Node::Node(Symbol const& symbol)
    : data_{symbol.data_}
{
}

Node&
Node::operator = (Node const& rhs)
{
    data_ = rhs.data_;
    return *this;
}

Node&
Node::operator = (List const& rhs)
{
    data_ = rhs.data_;
    return *this;
}

Node&
Node::operator = (Proc const& rhs)
{
    data_ = rhs.data_;
    return *this;
}

Node&
Node::operator = (Number const& rhs)
{
    data_ = rhs.data_;
    return *this;
}

Node&
Node::operator = (String const& rhs)
{
    data_ = rhs.data_;
    return *this;
}

Node&
Node::operator = (Symbol const& rhs)
{
    data_ = rhs.data_;
    return *this;
}

void
Node::accept(NodeVisitor& visitor) const
{
    if (data_) {
        data_->accept(visitor);
    } else {
        visitor.visit(nil);
    }
}

std::shared_ptr<Node::Data> const&
Node::data() const
{
    return data_;
}

////////////////////////////////////////////////////////////////////////////////
// List

List::List(Node head, List tail)
    : data_{ std::make_shared<Data>(head, tail) }
{
}

List::List(List const& other)
    : data_{other.data_}
{
}

List::List(std::shared_ptr<Data> const& data)
    : data_{data}
{
}

bool
List::empty() const
{
    return !data_;
}

Node
List::head() const
{
    return data_ ? data_->head : nil;
}

List
List::tail() const
{
    return data_ ? data_->tail : nil;
}


////////////////////////////////////////////////////////////////////////////////
// Proc

Proc::Proc(Func func)
    : data_{ std::make_shared<Data>(func) }
{
}

Proc::Proc(Proc const& other)
    : data_{other.data_}
{
}

Proc::Proc(std::shared_ptr<Data> data)
    : data_{data}
{
}

Node
Proc::call(List args, Env& env) const
{
    if (data_->func) {
        return data_->func(args, env);
    }
    return nil;
}

////////////////////////////////////////////////////////////////////////////////
// Number

Number::Number(double value)
    : data_{ std::make_shared<Data>(value) }
{
}

Number::Number(Number const& other)
    : data_{other.data_}
{
}

Number::Number(std::shared_ptr<Data> data)
    : data_{data}
{
}

double
Number::value() const
{
    return data_->value;
}

////////////////////////////////////////////////////////////////////////////////
// String

String::String(std::string text)
    : data_{ std::make_shared<Data>(std::move(text)) }
{
}

String::String(String const& other)
    : data_{other.data_}
{
}

String::String(std::shared_ptr<Data> data)
    : data_{data}
{
}

std::string const&
String::text() const
{
    return data_->text;
}

////////////////////////////////////////////////////////////////////////////////
// Symbol

Symbol::Symbol(std::string name)
{
    thread_local std::map<std::string, std::shared_ptr<Data>> symbols;

    auto i = symbols.find(name);
    if (i == symbols.end()) {
        data_ = std::make_shared<Data>(std::move(name));
        symbols.insert(std::make_pair(data_->name, data_));
    } else {
        data_ = i->second;
    }
}

Symbol::Symbol(Symbol const& other)
    : data_{other.data_}
{
}

Symbol::Symbol(std::shared_ptr<Data> data)
    : data_{data}
{
}

std::string const&
Symbol::name() const
{
    return data_->name;
}

////////////////////////////////////////////////////////////////////////////////
// Casting functions

std::optional<List>
to_list(Node const& node)
{
    if (!node.data()) {
        return nil;
    }

    auto data = std::dynamic_pointer_cast<List::Data>(node.data());
    if (data) {
        return { List{ data } };
    }

    return {};
}

std::optional<Proc>
to_proc(Node const& node)
{
    auto data = std::dynamic_pointer_cast<Proc::Data>(node.data());
    if (data) {
        return { Proc{ data } };
    }

    return {};
}

std::optional<Number>
to_number(Node const& node)
{
    auto data = std::dynamic_pointer_cast<Number::Data>(node.data());
    if (data) {
        return { Number{ data } };
    }

    return {};
}

std::optional<String>
to_string(Node const& node)
{
    auto data = std::dynamic_pointer_cast<String::Data>(node.data());
    if (data) {
        return { String{ data } };
    }

    return {};
}

std::optional<Symbol>
to_symbol(Node const& node)
{
    auto data = std::dynamic_pointer_cast<Symbol::Data>(node.data());
    if (data) {
        return { Symbol{ data } };
    }

    return {};
}

} // namespace mll