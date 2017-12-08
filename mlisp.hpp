#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

namespace mlisp {

    namespace detail {
        template <typename T>
        using Stack = std::stack<T, std::vector<T>>;

        template <int TAG>
        class UniqueRuntimeError: public std::runtime_error {
        public:
            explicit UniqueRuntimeError(char const* what) : std::runtime_error{what} {}
            explicit UniqueRuntimeError(std::string const& what) : std::runtime_error{what} {}
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
        Node(std::shared_ptr<Data const>) noexcept;

        Node& operator = (Node const&) noexcept;

        operator bool() const noexcept;

        void accept(NodeVisitor&) const;

        List to_list() const noexcept;
        Proc to_proc() const noexcept;
        Number to_number() const noexcept;
        Symbol to_symbol() const noexcept;

    protected:
        std::shared_ptr<Data const> data_;
    };

    class List: public Node {
    public:
        struct Data;

        List() noexcept;
        List(List const&) noexcept;
        List(std::shared_ptr<Data const>) noexcept;

        List& operator = (List const&) noexcept;

        friend Node car(List list) noexcept;
        friend List cdr(List list) noexcept;
    };

    using Func = std::function<Node(List, List)>;

    class Proc: public Node {
    public:
        Proc(Func func) noexcept;
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
        Number(double) noexcept;
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
        Symbol(std::string) noexcept;
        Symbol(Symbol const&) noexcept;

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

    using ParseError = detail::UniqueRuntimeError<0>;
    
    class Parser {
    public:
        Node parse(std::istream& istream);
        bool clean() const noexcept;

    private:
        struct Context {
            bool paren;
            Node head;
        };
        detail::Stack<Context> stack_;
    };

    using EvalError = detail::UniqueRuntimeError<1>;

    Node eval(Node expr, List env);

    Node car(List list) noexcept;
    List cdr(List list) noexcept;
    List cons(Node head, List tail) noexcept;
}

namespace std {
    ostream& operator << (ostream& os, mlisp::Node const& node);
}
