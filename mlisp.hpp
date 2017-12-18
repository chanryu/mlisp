#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

namespace mlisp {

    class Node;
    class List;
    class Proc;
    class Number;
    class String;
    class Symbol;

    class Env;
    using Func = std::function<Node(List, Env)>;

    class NodeVisitor {
    public:
        virtual void visit(List) = 0;
        virtual void visit(Proc) = 0;
        virtual void visit(Number) = 0;
        virtual void visit(String) = 0;
        virtual void visit(Symbol) = 0;
    };

    template <typename T> class Optional;

    class Node final {
    public:
        Node() noexcept;
        Node(Node const&) noexcept;
        Node(List const&) noexcept;
        Node(Proc const&) noexcept;
        Node(Number const&) noexcept;
        Node(String const&) noexcept;
        Node(Symbol const&) noexcept;

        Node& operator = (Node const&) noexcept;
        Node& operator = (List const&) noexcept;
        Node& operator = (Proc const&) noexcept;
        Node& operator = (Number const&) noexcept;
        Node& operator = (String const&) noexcept;
        Node& operator = (Symbol const&) noexcept;

        operator bool() const noexcept;

        void accept(NodeVisitor&);

        struct Data;
        friend Optional<List> to_list(Node) noexcept;
        friend Optional<Proc> to_proc(Node) noexcept;
        friend Optional<Number> to_number(Node) noexcept;
        friend Optional<String> to_string(Node) noexcept;
        friend Optional<Symbol> to_symbol(Node) noexcept;

    private:
        std::shared_ptr<Data> data_;

    private:
        bool operator == (Node const&) noexcept = delete;
    };

    class List final {
    public:
        List() noexcept;
        List(Node head, List tail) noexcept;
        List(List const&) noexcept;

        operator bool() const noexcept;

        Node head() const;
        List tail() const;

    public:
        struct Data;
        friend class Node;
        friend Node car(List) noexcept;
        friend List cdr(List) noexcept;
        friend Optional<List> to_list(Node) noexcept;

    private:
        List(std::shared_ptr<Data>) noexcept;
        std::shared_ptr<Data> data_;
    };

    class Proc final {
    public:
        explicit Proc(Func) noexcept;
        Proc(Proc const&) noexcept;

        Node operator()(List, Env) const;

    public:
        struct Data;
        friend class Node;
        friend Optional<Proc> to_proc(Node) noexcept;

    private:
        Proc(std::shared_ptr<Data>) noexcept;
        std::shared_ptr<Data> data_;
    };

    class Number final {
    public:
        explicit Number(double) noexcept;
        Number(Number const&) noexcept;

        double value() const;

    public:
        struct Data;
        friend class Node;
        friend Optional<Number> to_number(Node) noexcept;

    private:
        Number(std::shared_ptr<Data>) noexcept;
        std::shared_ptr<Data> data_;
    };

    class String final {
    public:
        explicit String(std::string) noexcept;
        String(String const&) noexcept;

        std::string const& text() const;

    public:
        struct Data;
        friend class Node;
        friend Optional<String> to_string(Node) noexcept;

    private:
        String(std::shared_ptr<Data>) noexcept;
        std::shared_ptr<Data> data_;
    };

    class Symbol final {
    public:
        explicit Symbol(std::string) noexcept;
        Symbol(Symbol const&) noexcept;

        std::string const& name() const;

    public:
        struct Data;
        friend class Node;
        friend Optional<Symbol> to_symbol(Node) noexcept;

    private:
        Symbol(std::shared_ptr<Data>) noexcept;
        std::shared_ptr<Data> data_;
    };
}

namespace mlisp {

    // Parser

    class Parser {
    public:
        Optional<Node> parse(std::istream&);
        bool clean() const noexcept;

    protected:
        virtual std::string translate(std::string token) const;

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

    // Env & eval

    class Env {
    public:
        Env();

        Env derive_new() const;

        void set(std::string const&, Node);
        bool update(std::string const&, Node);
        Optional<Node> lookup(std::string const&) const;
        Optional<Node> shallow_lookup(std::string const&) const;

    private:
        struct Data;
        std::shared_ptr<Data> data_;
    };

    Node eval(Node expr, Env env); // throws EvalError
}

namespace mlisp {

    // Overloaded ostream << operators

    std::ostream& operator << (std::ostream& os, Node const&);
    std::ostream& operator << (std::ostream& os, List const&);
}

namespace std {
    string to_string(mlisp::Node const&);
}

namespace mlisp {

    // Casting functions

    Optional<List> to_list(Node) noexcept;
    Optional<Proc> to_proc(Node) noexcept;
    Optional<Number> to_number(Node) noexcept;
    Optional<String> to_string(Node) noexcept;
    Optional<Symbol> to_symbol(Node) noexcept;
}

namespace mlisp {

    // Exceptions

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
        using UniqueRuntimeError::UniqueRuntimeError;
    };

    struct EvalError: detail::UniqueRuntimeError<1> {
        using UniqueRuntimeError::UniqueRuntimeError;
    };
}

namespace mlisp {

    // Convience wrappers

    inline List make_pair(Node head, List tail) noexcept
    {
        return List{ head, tail };
    }

    inline Proc make_proc(Func func) noexcept
    {
        return Proc{ func };
    }

    inline Number make_number(double value) noexcept
    {
        return Number{ value };
    }

    inline String make_string(std::string text) noexcept
    {
        return String{ std::move(text) };
    }

    inline Symbol make_symbol(std::string name) noexcept
    {
        return Symbol{ std::move(name) };
    }

    inline List cons(Node head, List tail) noexcept
    {
        return make_pair(head, tail);
    }

    inline Node car(List list) noexcept
    {
        return list.head();
    }

    inline List cdr(List list) noexcept
    {
        return list.tail();
    }
}

namespace mlisp {

    // Optional template

    template <typename T> class Optional {
    public:
        Optional() noexcept {}
        Optional(T t) noexcept : t_{ std::make_shared<T>(t) } {}

        operator bool() const noexcept { return !!t_; }

        T& operator *() const { return *t_; }
        T* operator ->() const { return t_.get(); }

    private:
        std::shared_ptr<T> t_;
    };
}
