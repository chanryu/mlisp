#include <mll/proc.hpp>

#include <mll/env.hpp>

namespace mll {

Proc::Proc(std::string name, Func func) : _core{std::make_shared<Core>(std::move(name), std::move(func))}
{}

Proc::Proc(std::string name, ClosureFunc func, std::shared_ptr<Env> const& outer_env)
    : _core{std::make_shared<Core>(std::move(name), std::move(func), outer_env)}
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
    else if (_core->closure_func) {
        return _core->closure_func(args, env, *_core->outer_env);
    }
    else {
        return {};
    }
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

Proc::Core::Core(std::string n, Func f) : name{std::move(n)}, func{std::move(f)}
{}

Proc::Core::Core(std::string n, ClosureFunc f, std::shared_ptr<Env> const& e)
    : name{std::move(n)}, closure_func{std::move(f)}, outer_env{e}
{}

void Proc::Core::accept(NodeVisitor& visitor)
{
    visitor.visit(Proc{std::static_pointer_cast<Core>(shared_from_this())});
}

void Proc::Core::mark_reachables()
{
    Node::Core::mark_reachables();

    if (outer_env) {
        outer_env->mark_reachables();
    }
}

} // namespace mll