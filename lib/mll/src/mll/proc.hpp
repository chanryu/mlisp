#pragma once

#include <mll/node.hpp>

#include <functional>

namespace mll {

class Env;
using Func = std::function<Node(List const& /*args*/, Env& /*env*/)>;
using ClosureFunc = std::function<Node(List const& /*args*/, Env& /*env*/, Env& /*outer_env*/)>;

class Proc final {
public:
    Proc(std::string name, Func);
    Proc(std::string name, ClosureFunc, std::shared_ptr<Env> const& outer_env);
    Proc(Proc const&);

    const std::string& name() const;
    Node call(List const&, Env&) const;

    struct Core;
    std::shared_ptr<Core> const& core() const;

    static std::optional<Proc> from_node(Node const&);

private:
    Proc(std::shared_ptr<Core>);
    std::shared_ptr<Core> _core;
};

struct Proc::Core : Node::Core {
    Core(std::string, Func);
    Core(std::string, ClosureFunc, std::shared_ptr<Env> const&);
    void accept(NodeVisitor& visitor) final;
    void mark_reachables() final;

    std::string const name;
    Func const func;
    ClosureFunc const closure_func;
    std::shared_ptr<Env> const outer_env;
};
} // namespace mll