#pragma once

#include <mll/node.hpp>

#include <functional>

namespace mll {

class Env;
using Func = std::function<Node(List const&, Env&)>;

class Proc final {
public:
    Proc(std::string name, Func);
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
    void accept(NodeVisitor& visitor) final;

    std::string const name;
    Func const func;
};
} // namespace mll