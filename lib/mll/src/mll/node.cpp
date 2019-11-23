#include <mll/node.hpp>

#include <mll/custom.hpp>
#include <mll/list.hpp>
#include <mll/proc.hpp>
#include <mll/symbol.hpp>

namespace mll {

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

} // namespace mll