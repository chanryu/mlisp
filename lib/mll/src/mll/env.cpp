#include <mll/env.hpp>

#include <mll/node.hpp>
#include <mll/quote.hpp>

namespace mll {

std::shared_ptr<Env> Env::create()
{
    struct Env_ : Env {};
    auto env = std::make_shared<Env_>();
    load_quote_procs(*env);
    return env;
}

std::shared_ptr<Env> Env::derive_new()
{
    auto derived = create();
    derived->base_ = shared_from_this();
    return derived;
}

void Env::set(std::string const& name, Node const& value)
{
    vars_[name] = value;
}

bool Env::deep_update(std::string const& name, Node const& value)
{
    for (auto env = this; env; env = env->base_.get()) {
        if (auto it = env->vars_.find(name); it != env->vars_.end()) {
            it->second = value;
            return true;
        }
    }
    return false;
}

bool Env::shallow_update(std::string const& name, Node const& value)
{
    if (auto it = vars_.find(name); it != vars_.end()) {
        it->second = value;
        return true;
    }
    return false;
}

std::optional<Node> Env::deep_lookup(std::string const& name) const
{
    for (auto env = this; env; env = env->base_.get()) {
        if (auto it = env->vars_.find(name); it != env->vars_.end()) {
            return it->second;
        }
    }
    return std::nullopt;
}

std::optional<Node> Env::shallow_lookup(std::string const& name) const
{
    if (auto it = vars_.find(name); it != vars_.end()) {
        return it->second;
    }
    return std::nullopt;
}

} // namespace mll