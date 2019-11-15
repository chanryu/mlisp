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
    derived->_base = shared_from_this();
    return derived;
}

void Env::set(std::string const& name, Node const& value)
{
    _vars[name] = value;
}

bool Env::deep_update(std::string const& name, Node const& value)
{
    for (auto env = this; env; env = env->_base.get()) {
        if (auto it = env->_vars.find(name); it != env->_vars.end()) {
            it->second = value;
            return true;
        }
    }
    return false;
}

bool Env::shallow_update(std::string const& name, Node const& value)
{
    if (auto it = _vars.find(name); it != _vars.end()) {
        it->second = value;
        return true;
    }
    return false;
}

std::optional<Node> Env::deep_lookup(std::string const& name) const
{
    for (auto env = this; env; env = env->_base.get()) {
        if (auto it = env->_vars.find(name); it != env->_vars.end()) {
            return it->second;
        }
    }
    return std::nullopt;
}

std::optional<Node> Env::shallow_lookup(std::string const& name) const
{
    if (auto it = _vars.find(name); it != _vars.end()) {
        return it->second;
    }
    return std::nullopt;
}

} // namespace mll