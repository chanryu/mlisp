#pragma once

#include <functional>
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

class Env;
using Func = std::function<Node(List const&, Env&)>;

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

class List final {
public:
    List() = default;

    List(List const&);
    List(Node const& head, List const& tail);

    bool empty() const;

    Node head() const;
    List tail() const;

    struct Core;
    std::shared_ptr<Core> const& core() const;

    static std::optional<List> from_node(Node const&);

private:
    List(std::shared_ptr<Core> const&);
    std::shared_ptr<Core> _core;
};

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

// The nil
extern List const nil;

// Helper functions

template <typename T>
inline std::optional<T> dynamic_node_cast(Node const& node)
{
    return T::from_node(node);
}

inline List cons(Node const& head, List const& tail)
{
    return List{head, tail};
}

inline Node car(List const& list)
{
    return list.head();
}

inline List cdr(List const& list)
{
    return list.tail();
}

inline Node cadr(List const& list)
{
    return car(cdr(list));
}

template <typename Func>
List map(List list, Func func)
{
    std::stack<Node> stack;
    while (!list.empty()) {
        stack.push(func(car(list)));
        list = cdr(list);
    }
    while (!stack.empty()) {
        list = cons(stack.top(), list);
        stack.pop();
    }
    return list;
}

} // namespace mll
