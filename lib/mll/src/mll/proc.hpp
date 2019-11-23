#pragma once

#include <mll/node.hpp>

#include <functional>

namespace mll {

class Env;
using Func = std::function<Node(List const&, Env&)>;

class Proc final {
public:
    explicit Proc(Func);
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
    explicit Core(std::string n, Func f) : name{std::move(n)}, func{std::move(f)}
    {}

    void accept(NodeVisitor& visitor) final
    {
        visitor.visit(Proc{std::static_pointer_cast<Core>(shared_from_this())});
    }

    std::string const name;
    Func const func;
};
} // namespace mll