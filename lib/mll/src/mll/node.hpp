#pragma once

#include <memory>
#include <optional>
#include <stack>
#include <string>

namespace mll {

class Node;
class List;
class Proc;
class Symbol;
class Custom;

class NodeVisitor {
public:
    virtual void visit(List const&) = 0;
    virtual void visit(Proc const&) = 0;
    virtual void visit(Symbol const&) = 0;
    virtual void visit(Custom const&) = 0;
};

class Node final {
public:
    Node() = default;

    Node(Node const&);
    Node(List const&);
    Node(Proc const&);
    Node(Symbol const&);
    Node(Custom const&);

    Node& operator=(Node const&);
    Node& operator=(List const&);
    Node& operator=(Proc const&);
    Node& operator=(Symbol const&);
    Node& operator=(Custom const&);

    void accept(NodeVisitor&) const;

    struct Core : std::enable_shared_from_this<Core> {
        virtual ~Core() = default;
        virtual void accept(NodeVisitor&) = 0;
    };
    std::shared_ptr<Core> const& core() const;

private:
    std::shared_ptr<Core> _core;
};

template <typename T>
inline std::optional<T> dynamic_node_cast(Node const& node)
{
    return T::from_node(node);
}

} // namespace mll
