#pragma once

#include <mll/node.hpp>

namespace mll {

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

struct List::Core : Node::Core {
    Core(Node const& h, List const& t);
    void accept(NodeVisitor& visitor) final;

    Node const head;
    List const tail;
};

// The nil
extern List const nil;

// Helper functions

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

inline size_t length(List list)
{
    size_t len = 0;
    while (!list.empty()) {
        len += 1;
        list = cdr(list);
    }
    return len;
}

template <typename Func>
void for_each(List list, Func&& func)
{
    while (!list.empty()) {
        func(car(list));
        list = cdr(list);
    }
}

template <typename Func>
void for_each_with_index(List list, Func const& func)
{
    for (size_t i = 0; !list.empty(); ++i) {
        func(i, car(list));
        list = cdr(list);
    }
}

template <typename Func>
List map(List list, Func&& func)
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