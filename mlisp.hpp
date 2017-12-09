#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

namespace mlisp {

    namespace detail {
        template <int TAG>
        class UniqueRuntimeError: public std::runtime_error {
        public:
            explicit UniqueRuntimeError(char const* what)
                : std::runtime_error{what} {}
            explicit UniqueRuntimeError(std::string const& what)
                : std::runtime_error{what} {}
        };
    }

    class NodeVisitor;

    class List;
    class Proc;
    class Number;
    class Symbol;

    class Node {
    public:
        struct Data;

        Node() noexcept;
        Node(Node const&) noexcept;

        Node& operator = (Node const&) noexcept;
        bool operator == (Node const&) noexcept = delete;

        operator bool() const noexcept;

        void accept(NodeVisitor&) const;

        List to_list() const;
        Proc to_proc() const;
        Number to_number() const;
        Symbol to_symbol() const;

    protected:
        Node(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;
    };

    class List: public Node {
    public:
        List() noexcept;
        List(List const&) noexcept;

        List& operator = (List const&) noexcept;

        friend Node car(List list) noexcept;
        friend List cdr(List list) noexcept;
        friend List cons(Node head, List tail) noexcept;

    private:
        struct Data;
        friend class Node;
        List(std::shared_ptr<Data const>) noexcept;
    };

    Node car(List list) noexcept;
    List cdr(List list) noexcept;
    List cons(Node head, List tail) noexcept;

    using Func = std::function<Node(List, List)>;

    class Proc: public Node {
    public:
        explicit Proc(Func func) noexcept;
        Proc(Proc const&) noexcept;

        Node operator()(List, List) const;

    private:
        struct Data;
        friend class Node;
        friend struct Data;
        Proc(std::shared_ptr<Data const>) noexcept;
    };

    class Number: public Node {
    public:
        explicit Number(double) noexcept;
        Number(Number const&) noexcept;

        double value() const;

    private:
        struct Data;
        friend class Node;
        friend struct Data;
        Number(std::shared_ptr<Data const>) noexcept;
    };

    class Symbol: public Node {
    public:
        explicit Symbol(std::string) noexcept;
        Symbol(Symbol const&) noexcept;

        bool operator == (Node const&) noexcept;

        std::string const& name() const;

    private:
        struct Data;
        friend class Node;
        friend struct Data;
        Symbol(std::shared_ptr<Data const>) noexcept;
    };

    class NodeVisitor {
    public:
        virtual void visit(List) = 0;
        virtual void visit(Proc) = 0;
        virtual void visit(Number) = 0;
        virtual void visit(Symbol) = 0;
    };

    struct ParseError: detail::UniqueRuntimeError<0> {
        using UniqueRuntimeError<0>::UniqueRuntimeError;
    };
    
    class Parser {
    public:
        bool parse(std::istream& istream, Node& expr);
        bool clean() const noexcept;

    private:
        struct Context {
            bool paren_open;
            bool head_empty;
            Node head;
        };
        std::stack<Context> stack_;
    };

    struct EvalError: detail::UniqueRuntimeError<1> {
        using UniqueRuntimeError<1>::UniqueRuntimeError;
    };

    struct TypeError: EvalError {
        using EvalError::EvalError;
    };

    Node eval(Node expr, List env);
}

namespace std {
    ostream& operator << (ostream& os, mlisp::Node const& node);
}
