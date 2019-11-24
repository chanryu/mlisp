#include <mll/proc.hpp>

namespace mll {

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
    return {};
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

} // namespace mll