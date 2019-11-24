#include <mll/list.hpp>

namespace mll {

List const nil;

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

List::Core::Core(Node const& h, List const& t) : head{h}, tail{t}
{}

void List::Core::accept(NodeVisitor& visitor)
{
    visitor.visit(List{std::static_pointer_cast<Core>(shared_from_this())});
}

} // namespace mll