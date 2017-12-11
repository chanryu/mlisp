#include <iostream>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

namespace mlisp {

    namespace detail {
        struct NodeData;
        struct ListData;
        struct ProcData;
        struct NumberData;
        struct SymbolData;
    }

    class Node;
    class List;
    class Proc;
    class Number;
    class Symbol;

    using Func = std::function<Node(List, List)>;

    Node car(List) noexcept;
    List cdr(List) noexcept;
    List cons(Node head, List tail) noexcept;
    Proc proc(Func) noexcept;
    Number number(double) noexcept;
    Symbol symbol(std::string) noexcept;

    class NodeVisitor;

    class Node final {
    public:
        Node() noexcept;
        Node(Node const&) noexcept;
        Node(List const&) noexcept;
        Node(Proc const&) noexcept;
        Node(Number const&) noexcept;
        Node(Symbol const&) noexcept;

        Node& operator = (Node const&) noexcept;
        Node& operator = (List const&) noexcept;
        Node& operator = (Proc const&) noexcept;
        Node& operator = (Number const&) noexcept;
        Node& operator = (Symbol const&) noexcept;

        operator bool() const noexcept;

        void accept(NodeVisitor&) const;

        List to_list() const;
        Proc to_proc() const;
        Number to_number() const;
        Symbol to_symbol() const;

    private:
        typedef detail::NodeData Data;
        Node(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;

    private:
        bool operator == (Node const&) noexcept = delete;
    };

    class List final {
    public:
        List() noexcept;
        List(List const&) noexcept;

        List& operator = (List const&) noexcept;

        operator bool() const noexcept;

    private:
        friend class Node;
        friend struct detail::ListData;
        friend Node car(List list) noexcept;
        friend List cdr(List list) noexcept;
        friend List cons(Node head, List tail) noexcept;

        typedef detail::ListData Data;
        List(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;
    };

    class Proc final {
    public:
        Proc(Proc const&) noexcept;

        Node operator()(List, List) const;

    private:
        friend class Node;
        friend struct detail::ProcData;
        friend Proc proc(Func) noexcept;

        typedef detail::ProcData Data;
        Proc(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;
    };

    class Number final {
    public:
        Number(Number const&) noexcept;

        double value() const;

    private:
        friend class Node;
        friend struct detail::NumberData;
        friend Number number(double) noexcept;

        typedef detail::NumberData Data;
        Number(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;
    };

    class Symbol final {
    public:
        Symbol(Symbol const&) noexcept;

        bool operator == (Node const&) noexcept;

        std::string const& name() const;

    private:
        friend class Node;
        friend struct detail::SymbolData;
        friend Symbol symbol(std::string) noexcept;

        typedef detail::SymbolData Data;
        Symbol(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;
    };

    class NodeVisitor {
    public:
        virtual void visit(List) = 0;
        virtual void visit(Proc) = 0;
        virtual void visit(Number) = 0;
        virtual void visit(Symbol) = 0;
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
}

namespace mlisp {
    class NodeEvaluator: NodeVisitor {
    public:
        explicit NodeEvaluator(List env);

        Node evaluate(Node expr);

    private:
        void visit(List list) override;
        void visit(Proc proc) override;
        void visit(Number number) override;
        void visit(Symbol symbol) override;

    private:
        List env_;
        Node result_;
    };

    Node eval(Node expr, List env); // throws EvalError
}

namespace mlisp {
    // printing helper
    std::ostream& operator << (std::ostream& os, Node const&);
    std::ostream& operator << (std::ostream& os, List const&);
}

namespace std {
    string to_string(mlisp::Node const&);
}

namespace mlisp {
    // exceptions
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

    struct ParseError: detail::UniqueRuntimeError<0> {
        using UniqueRuntimeError<0>::UniqueRuntimeError;
    };

    struct EvalError: detail::UniqueRuntimeError<1> {
        using UniqueRuntimeError<1>::UniqueRuntimeError;
    };
}
