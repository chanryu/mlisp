#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace mll {

class Node;
class List;
class Proc;
class Number;
class String;
class Symbol;

class Env;
using Func = std::function<Node(List const&, Env&)>;

class NodeVisitor {
public:
    virtual void visit(List const&) = 0;
    virtual void visit(Proc const&) = 0;
    virtual void visit(Number const&) = 0;
    virtual void visit(String const&) = 0;
    virtual void visit(Symbol const&) = 0;
};

class Node final {
public:
    Node() = default;

    Node(Node const&);
    Node(List const&);
    Node(Proc const&);
    Node(Number const&);
    Node(String const&);
    Node(Symbol const&);

    Node& operator = (Node const&);
    Node& operator = (List const&);
    Node& operator = (Proc const&);
    Node& operator = (Number const&);
    Node& operator = (String const&);
    Node& operator = (Symbol const&);

    void accept(NodeVisitor&) const;

    struct Data;
    std::shared_ptr<Data> const& data() const;

private:
    bool operator == (Node const&) const = delete;
    std::shared_ptr<Data> data_;
};

class List final {
public:
    List() = default;

    List(Node head, List tail);
    List(List const&);

    bool empty() const;

    Node head() const;
    List tail() const;

public:
    struct Data;
    friend class Node;
    friend std::optional<List> to_list(Node const&);

private:
    List(std::shared_ptr<Data> const&);
    std::shared_ptr<Data> data_;
};

class Proc final {
public:
    explicit Proc(Func);
    Proc(Proc const&);

    Node call(List, Env&) const;

public:
    struct Data;
    friend class Node;
    friend std::optional<Proc> to_proc(Node const&);

private:
    Proc(std::shared_ptr<Data>);
    std::shared_ptr<Data> data_;
};

class Number final {
public:
    explicit Number(double);
    Number(Number const&);

    double value() const;

public:
    struct Data;
    friend class Node;
    friend std::optional<Number> to_number(Node const&);

private:
    Number(std::shared_ptr<Data>);
    std::shared_ptr<Data> data_;
};

class String final {
public:
    explicit String(std::string);
    String(String const&);

    std::string const& text() const;

public:
    struct Data;
    friend class Node;
    friend std::optional<String> to_string(Node const&);

private:
    String(std::shared_ptr<Data>);
    std::shared_ptr<Data> data_;
};

class Symbol final {
public:
    explicit Symbol(std::string);
    Symbol(Symbol const&);

    std::string const& name() const;

public:
    struct Data;
    friend class Node;
    friend std::optional<Symbol> to_symbol(Node const&);

private:
    Symbol(std::shared_ptr<Data>);
    std::shared_ptr<Data> data_;
};

// casting functions
std::optional<List>   to_list(Node const&);
std::optional<Proc>   to_proc(Node const&);
std::optional<Number> to_number(Node const&);
std::optional<String> to_string(Node const&);
std::optional<Symbol> to_symbol(Node const&);

// the nil
extern List const nil;

// Convience wrappers

inline Proc make_proc(Func func)
{
    return Proc{ func };
}

inline Number make_number(double value)
{
    return Number{ value };
}

inline String make_string(std::string text)
{
    return String{ std::move(text) };
}

inline Symbol make_symbol(std::string name)
{
    return Symbol{ std::move(name) };
}

inline List cons(Node const& head, List const& tail)
{
    return List{ head, tail };
}

inline Node car(List const& list)
{
    return list.head();
}

inline List cdr(List const& list)
{
    return list.tail();
}

} // namespace mll
