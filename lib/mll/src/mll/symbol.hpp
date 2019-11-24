#pragma once

#include <mll/node.hpp>

namespace mll {

class Symbol final {
public:
    explicit Symbol(std::string);
    Symbol(Symbol const&);

    std::string const& name() const;

    struct Core;
    std::shared_ptr<Core> const& core() const;

    static std::optional<Symbol> from_node(Node const&);

private:
    Symbol(std::shared_ptr<Core>);
    std::shared_ptr<Core> _core;
};

struct Symbol::Core : Node::Core {
    explicit Core(std::string n) : name{std::move(n)}
    {}

    void accept(NodeVisitor& visitor) final
    {
        visitor.visit(Symbol{std::static_pointer_cast<Core>(shared_from_this())});
    }

    std::string const name;
};
} // namespace mll