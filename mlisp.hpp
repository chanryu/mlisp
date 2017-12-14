#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

namespace mlisp {

    namespace detail {
        struct NodeData;
        struct ConsData;
        struct NumberData;
        struct StringData;
        struct SymbolData;
        struct ProcedureData;
    }

    class Node;
    class Cons;
    class Procedure;
    class Number;
    class String;
    class Symbol;

    struct Env;
    using EnvPtr = std::shared_ptr<Env>;
    using Func = std::function<Node(Cons, EnvPtr)>;

    Cons cons(Node, Cons) noexcept;
    Number number(double) noexcept;
    String string(std::string) noexcept;
    Symbol symbol(std::string) noexcept;
    Procedure procedure(Func) noexcept;

    Node car(Cons) noexcept;
    Cons cdr(Cons) noexcept;

    class NodeVisitor {
    public:
        virtual void visit(Cons) = 0;
        virtual void visit(Number) = 0;
        virtual void visit(String) = 0;
        virtual void visit(Symbol) = 0;
        virtual void visit(Procedure) = 0;
    };

    class Node final {
    public:
        Node() noexcept;
        Node(Node const&) noexcept;
        Node(Cons const&) noexcept;
        Node(Number const&) noexcept;
        Node(String const&) noexcept;
        Node(Symbol const&) noexcept;
        Node(Procedure const&) noexcept;

        Node& operator = (Node const&) noexcept;
        Node& operator = (Cons const&) noexcept;
        Node& operator = (Number const&) noexcept;
        Node& operator = (String const&) noexcept;
        Node& operator = (Symbol const&) noexcept;
        Node& operator = (Procedure const&) noexcept;

        operator bool() const noexcept;

        void accept(NodeVisitor&) const;

        Cons to_cons() const;
        Number to_number() const;
        String to_string() const;
        Symbol to_symbol() const;
        Procedure to_procedure() const;

        bool is_cons() const;
        bool is_number() const;
        bool is_string() const;
        bool is_symbol() const;
        bool is_procedure() const;

    private:
        typedef detail::NodeData Data;
        Node(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;

    private:
        bool operator == (Node const&) noexcept = delete;
    };

    class Cons final {
    public:
        Cons() noexcept;
        Cons(Cons const&) noexcept;

        Cons& operator = (Cons const&) noexcept;

        operator bool() const noexcept;

    private:
        friend class Node;
        friend struct detail::ConsData;
        friend Cons cons(Node, Cons) noexcept;
        friend Node car(Cons) noexcept;
        friend Cons cdr(Cons) noexcept;

        typedef detail::ConsData Data;
        Cons(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;
    };

    class Procedure final {
    public:
        Procedure(Procedure const&) noexcept;

        Node operator()(Cons, EnvPtr) const;

    private:
        friend class Node;
        friend struct detail::ProcedureData;
        friend Procedure procedure(Func) noexcept;

        typedef detail::ProcedureData Data;
        Procedure(std::shared_ptr<Data const>) noexcept;
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

    class String final {
    public:
        String(String const&) noexcept;

        std::string const& text() const;

    private:
        friend class Node;
        friend struct detail::StringData;
        friend String string(std::string) noexcept;

        typedef detail::StringData Data;
        String(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;
    };

    class Symbol final {
    public:
        Symbol(Symbol const&) noexcept;

        std::string const& name() const;

    private:
        friend class Node;
        friend struct detail::SymbolData;
        friend Symbol symbol(std::string) noexcept;

        typedef detail::SymbolData Data;
        Symbol(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;
    };
}

namespace mlisp {

    // parser

    class Parser {
    public:
        bool parse(std::istream&, Node&);
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

    // Env & eval

    EnvPtr make_env(EnvPtr base_env);
    void set(EnvPtr, std::string, Node);
    bool update(EnvPtr, std::string const&, Node);
    bool lookup(EnvPtr, std::string const&, Node&);

    Node eval(Node expr, EnvPtr env); // throws EvalError
}

namespace mlisp {
    // printing helper
    std::ostream& operator << (std::ostream& os, Node const&);
    std::ostream& operator << (std::ostream& os, Cons const&);
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
