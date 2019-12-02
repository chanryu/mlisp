#include <mll/symbol.hpp>

#include <map>

namespace mll {

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

Symbol::Core::Core(std::string n) : name{std::move(n)}
{}

void Symbol::Core::accept(NodeVisitor& visitor)
{
    visitor.visit(Symbol{std::static_pointer_cast<Core>(shared_from_this())});
}

} // namespace mll