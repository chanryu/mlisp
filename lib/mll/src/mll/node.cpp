#include <mll/node.hpp>

#include <mll/custom.hpp>
#include <mll/list.hpp>
#include <mll/proc.hpp>
#include <mll/symbol.hpp>

#include <iostream>

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

static int totalObjectCount = 0;

Node::Core::Core()
{
    totalObjectCount += 1;
    std::cerr << "object count = " << totalObjectCount << std::endl;
}

Node::Core::~Core()
{
    totalObjectCount -= 1;
    std::cerr << "object count = " << totalObjectCount << std::endl;
}

} // namespace mll