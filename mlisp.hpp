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

    struct NodeData;
    struct ListData;
    struct ProcData;
    struct NumberData;
    struct SymbolData;

    class Node {
    public:
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
        Node(std::shared_ptr<NodeData const>) noexcept;
        std::shared_ptr<NodeData const> data_;
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
        friend class Node;
        friend struct ListData;
        List(std::shared_ptr<ListData const>) noexcept;
    };

    using Func = std::function<Node(List, List)>;

    class Proc: public Node {
    public:
        Proc(Proc const&) noexcept;

        Node operator()(List, List) const;

    private:
        friend class Node;
        friend struct ProcData;
        friend Proc proc(Func) noexcept;
        Proc(std::shared_ptr<ProcData const>) noexcept;
    };

    class Number: public Node {
    public:
        Number(Number const&) noexcept;

        double value() const;

    private:
        friend class Node;
        friend struct NumberData;
        friend Number number(double) noexcept;
        Number(std::shared_ptr<NumberData const>) noexcept;
    };

    class Symbol: public Node {
    public:
        Symbol(Symbol const&) noexcept;

        bool operator == (Node const&) noexcept;

        std::string const& name() const;

    private:
        friend class Node;
        friend struct SymbolData;
        friend Symbol symbol(std::string) noexcept;
        Symbol(std::shared_ptr<SymbolData const>) noexcept;
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
            enum class Type { quote, paren, list };
            Type type;
            Node head;
            bool head_empty;
        };
        std::stack<Context> stack_;
    };

    Node car(List list) noexcept;
    List cdr(List list) noexcept;
    List cons(Node head, List tail) noexcept;
    Proc proc(Func) noexcept;
    Number number(double) noexcept;
    Symbol symbol(std::string) noexcept;

    struct EvalError: detail::UniqueRuntimeError<1> {
        using UniqueRuntimeError<1>::UniqueRuntimeError;
    };

    struct TypeError: EvalError {
        using EvalError::EvalError;
    };

    Node eval(Node expr, List env);

    std::ostream& operator << (std::ostream& os, Node const&);
}
