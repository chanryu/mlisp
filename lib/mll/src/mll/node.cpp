#include <mll/node.hpp>

#include <mll/custom.hpp>

#include <map>

namespace mll {

List const nil;

////////////////////////////////////////////////////////////////////////////////
// Node::Data

struct List::Core : Node::Core {
    Core(Node const& h, List const& t) : head{h}, tail{t}
    {}

    void accept(NodeVisitor& visitor) final
    {
        visitor.visit(List{std::static_pointer_cast<Core>(shared_from_this())});
    }

    Node const head;
    List const tail;
};

struct Proc::Core : Node::Core {
    explicit Core(std::string n, Func f) : name{std::move(n)}, func{std::move(f)}
    {}

    void accept(NodeVisitor& visitor) final
    {
        visitor.visit(Proc{std::static_pointer_cast<Core>(shared_from_this())});
    }

    std::string const name;
    Func const func;
};

struct Symbol::Core : Node::Core {
    explicit Core(std::string n) : name{std::move(n)}
    {}

    void accept(NodeVisitor& visitor) final
    {
        visitor.visit(Symbol{std::static_pointer_cast<Core>(shared_from_this())});
    }

    std::string const name;
};

////////////////////////////////////////////////////////////////////////////////
// Node

Node::Node(Node const& other) : _core{other._core}
{}

Node::Node(List const& list) : _core{list.core()}
{}

Node::Node(Proc const& proc) : _core{proc.core()}
{}

Node::Node(Symbol const& symbol) : _core{symbol.core()}
{}

Node::Node(Custom const& custom) : _core{custom.core()}
{}

Node& Node::operator=(Node const& rhs)
{
    _core = rhs._core;
    return *this;
}

Node& Node::operator=(List const& rhs)
{
    _core = rhs.core();
    return *this;
}

Node& Node::operator=(Proc const& rhs)
{
    _core = rhs.core();
    return *this;
}

Node& Node::operator=(Symbol const& rhs)
{
    _core = rhs.core();
    return *this;
}

Node& Node::operator=(Custom const& rhs)
{
    _core = rhs.core();
    return *this;
}

void Node::accept(NodeVisitor& visitor) const
{
    if (_core) {
        _core->accept(visitor);
    }
    else {
        visitor.visit(nil);
    }
}

std::shared_ptr<Node::Core> const& Node::core() const
{
    return _core;
}

////////////////////////////////////////////////////////////////////////////////
// List

List::List(List const& other) : _core{other._core}
{}

List::List(Node const& head, List const& tail) : _core{std::make_shared<Core>(head, tail)}
{}

List::List(std::shared_ptr<Core> const& core) : _core{core}
{}

bool List::empty() const
{
    return !_core;
}

Node List::head() const
{
    return _core ? _core->head : nil;
}

List List::tail() const
{
    return _core ? _core->tail : nil;
}

std::shared_ptr<List::Core> const& List::core() const
{
    return _core;
}

std::optional<List> List::from_node(Node const& node)
{
    if (!node.core()) {
        return nil;
    }

    if (auto core = std::dynamic_pointer_cast<Core>(node.core())) {
        return List{core};
    }
    return std::nullopt;
}

////////////////////////////////////////////////////////////////////////////////
// Proc

Proc::Proc(Func func) : _core{std::make_shared<Core>("anonymous", std::move(func))}
{}

Proc::Proc(std::string name, Func func) : _core{std::make_shared<Core>(std::move(name), std::move(func))}
{}

Proc::Proc(Proc const& other) : _core{other._core}
{}

Proc::Proc(std::shared_ptr<Core> core) : _core{core}
{}

const std::string& Proc::name() const
{
    return _core->name;
}

Node Proc::call(List const& args, Env& env) const
{
    if (_core->func) {
        return _core->func(args, env);
    }
    return nil;
}

std::shared_ptr<Proc::Core> const& Proc::core() const
{
    return _core;
}

std::optional<Proc> Proc::from_node(Node const& node)
{
    if (auto core = std::dynamic_pointer_cast<Core>(node.core())) {
        return Proc{core};
    }
    return std::nullopt;
}

////////////////////////////////////////////////////////////////////////////////
// Symbol

Symbol::Symbol(std::string name)
{
    thread_local std::map<std::string, std::shared_ptr<Core>> symbols;

    auto i = symbols.find(name);
    if (i == symbols.end()) {
        _core = std::make_shared<Core>(std::move(name));
        symbols.insert(std::make_pair(_core->name, _core));
    }
    else {
        _core = i->second;
    }
}

Symbol::Symbol(Symbol const& other) : _core{other._core}
{}

Symbol::Symbol(std::shared_ptr<Core> core) : _core{core}
{}

std::string const& Symbol::name() const
{
    return _core->name;
}

std::shared_ptr<Symbol::Core> const& Symbol::core() const
{
    return _core;
}

std::optional<Symbol> Symbol::from_node(Node const& node)
{
    if (auto core = std::dynamic_pointer_cast<Core>(node.core())) {
        return Symbol{core};
    }
    return std::nullopt;
}

} // namespace mll