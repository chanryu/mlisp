#include <mll/node.hpp>

#include <mll/custom.hpp>

#include <map>

namespace mll {

List const nil;

////////////////////////////////////////////////////////////////////////////////
// Node::Data

struct List::Data : Node::Data {
    Data(Node const& h, List const& t) : head{h}, tail{t}
    {}

    void accept(NodeVisitor& visitor) override
    {
        visitor.visit(List{std::static_pointer_cast<Data>(shared_from_this())});
    }

    Node const head;
    List const tail;
};

struct Proc::Data : Node::Data {
    explicit Data(std::string n, Func f) : name{std::move(n)}, func{std::move(f)}
    {}

    void accept(NodeVisitor& visitor) override
    {
        visitor.visit(Proc{std::static_pointer_cast<Data>(shared_from_this())});
    }

    std::string const name;
    Func const func;
};

struct Symbol::Data : Node::Data {
    explicit Data(std::string n) : name{std::move(n)}
    {}

    void accept(NodeVisitor& visitor) override
    {
        visitor.visit(Symbol{std::static_pointer_cast<Data>(shared_from_this())});
    }

    std::string const name;
};

////////////////////////////////////////////////////////////////////////////////
// Node

Node::Node(Node const& other) : _data{other._data}
{}

Node::Node(List const& list) : _data{list.data()}
{}

Node::Node(Proc const& proc) : _data{proc.data()}
{}

Node::Node(Symbol const& symbol) : _data{symbol.data()}
{}

Node::Node(Custom const& custom) : _data{custom.data()}
{}

Node& Node::operator=(Node const& rhs)
{
    _data = rhs._data;
    return *this;
}

Node& Node::operator=(List const& rhs)
{
    _data = rhs.data();
    return *this;
}

Node& Node::operator=(Proc const& rhs)
{
    _data = rhs.data();
    return *this;
}

Node& Node::operator=(Symbol const& rhs)
{
    _data = rhs.data();
    return *this;
}

Node& Node::operator=(Custom const& rhs)
{
    _data = rhs.data();
    return *this;
}

void Node::accept(NodeVisitor& visitor) const
{
    if (_data) {
        _data->accept(visitor);
    }
    else {
        visitor.visit(nil);
    }
}

std::shared_ptr<Node::Data> const& Node::data() const
{
    return _data;
}

////////////////////////////////////////////////////////////////////////////////
// List

List::List(List const& other) : _data{other._data}
{}

List::List(Node const& head, List const& tail) : _data{std::make_shared<Data>(head, tail)}
{}

List::List(std::shared_ptr<Data> const& data) : _data{data}
{}

bool List::empty() const
{
    return !_data;
}

Node List::head() const
{
    return _data ? _data->head : nil;
}

List List::tail() const
{
    return _data ? _data->tail : nil;
}

std::shared_ptr<List::Data> const& List::data() const
{
    return _data;
}

std::optional<List> List::from_node(Node const& node)
{
    if (!node.data()) {
        return nil;
    }

    if (auto data = std::dynamic_pointer_cast<Data>(node.data())) {
        return List{data};
    }
    return std::nullopt;
}

////////////////////////////////////////////////////////////////////////////////
// Proc

Proc::Proc(Func func) : _data{std::make_shared<Data>("anonymous", std::move(func))}
{}

Proc::Proc(std::string name, Func func) : _data{std::make_shared<Data>(std::move(name), std::move(func))}
{}

Proc::Proc(Proc const& other) : _data{other._data}
{}

Proc::Proc(std::shared_ptr<Data> data) : _data{data}
{}

const std::string& Proc::name() const
{
    return _data->name;
}

Node Proc::call(List const& args, Env& env) const
{
    if (_data->func) {
        return _data->func(args, env);
    }
    return nil;
}

std::shared_ptr<Proc::Data> const& Proc::data() const
{
    return _data;
}

std::optional<Proc> Proc::from_node(Node const& node)
{
    if (auto data = std::dynamic_pointer_cast<Data>(node.data())) {
        return Proc{data};
    }
    return std::nullopt;
}

////////////////////////////////////////////////////////////////////////////////
// Symbol

Symbol::Symbol(std::string name)
{
    thread_local std::map<std::string, std::shared_ptr<Data>> symbols;

    auto i = symbols.find(name);
    if (i == symbols.end()) {
        _data = std::make_shared<Data>(std::move(name));
        symbols.insert(std::make_pair(_data->name, _data));
    }
    else {
        _data = i->second;
    }
}

Symbol::Symbol(Symbol const& other) : _data{other._data}
{}

Symbol::Symbol(std::shared_ptr<Data> data) : _data{data}
{}

std::string const& Symbol::name() const
{
    return _data->name;
}

std::shared_ptr<Symbol::Data> const& Symbol::data() const
{
    return _data;
}

std::optional<Symbol> Symbol::from_node(Node const& node)
{
    if (auto data = std::dynamic_pointer_cast<Data>(node.data())) {
        return Symbol{data};
    }
    return std::nullopt;
}

} // namespace mll