#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

namespace mll {

    class Node;
    class Pair;
    class Proc;
    class Number;
    class String;
    class Symbol;

    class Env;
    using Func = std::function<Node(Pair, Env)>;

    class NodeVisitor {
    public:
        virtual void visit(Pair) = 0;
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
        Node(Pair const&) noexcept;
        Node(Proc const&) noexcept;
        Node(Number const&) noexcept;
        Node(String const&) noexcept;
        Node(Symbol const&) noexcept;

        Node& operator = (Node const&) noexcept;
        Node& operator = (Pair const&) noexcept;
        Node& operator = (Proc const&) noexcept;
        Node& operator = (Number const&) noexcept;
        Node& operator = (String const&) noexcept;
        Node& operator = (Symbol const&) noexcept;

        bool operator == (Node const&) noexcept;
        operator bool() const noexcept;

        void accept(NodeVisitor&);

        struct Data;
        friend Optional<Pair> to_pair(Node) noexcept;
        friend Optional<Proc> to_proc(Node) noexcept;
        friend Optional<Number> to_number(Node) noexcept;
        friend Optional<String> to_string(Node) noexcept;
        friend Optional<Symbol> to_symbol(Node) noexcept;

    private:
        std::shared_ptr<Data> data_;
    };

    class Pair final {
    public:
        Pair() noexcept;
        Pair(Node head, Pair tail) noexcept;
        Pair(Pair const&) noexcept;

        operator bool() const noexcept;

        Node head() const;
        Pair tail() const;

    public:
        struct Data;
        friend class Node;
        friend Node car(Pair) noexcept;
        friend Pair cdr(Pair) noexcept;
        friend Optional<Pair> to_pair(Node) noexcept;

    private:
        Pair(std::shared_ptr<Data>) noexcept;
        std::shared_ptr<Data> data_;
    };

    class Proc final {
    public:
        explicit Proc(Func) noexcept;
        Proc(Proc const&) noexcept;

        Node operator()(Pair, Env) const;

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

namespace mll {

    // Parser

    class Parser {
    public:
        Optional<Node> parse(std::istream&);
        bool clean() const noexcept;

    protected:
        virtual std::string translate(std::string token) const;

    private:
        bool get_token(std::istream& istream);
        std::string token_;
        bool token_escaped_;

        struct Context {
            enum class Type { quote, paren, list };
            Type type;
            Node head;
            bool head_empty;
        };
        std::stack<Context> stack_;
    };
}

namespace mll {

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

namespace mll {

    // Overloaded ostream << operators

    std::ostream& operator << (std::ostream& os, Node const&);
    std::ostream& operator << (std::ostream& os, Pair const&);
}

namespace std {
    string to_string(mll::Node const&);
}

namespace mll {

    // Casting functions

    Optional<Pair> to_pair(Node) noexcept;
    Optional<Proc> to_proc(Node) noexcept;
    Optional<Number> to_number(Node) noexcept;
    Optional<String> to_string(Node) noexcept;
    Optional<Symbol> to_symbol(Node) noexcept;
}

namespace mll {

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

namespace mll {

    // Convience wrappers

    inline Pair make_pair(Node head, Pair tail) noexcept
    {
        return Pair{ head, tail };
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

    inline Pair cons(Node head, Pair tail) noexcept
    {
        return make_pair(head, tail);
    }

    inline Node car(Pair pair) noexcept
    {
        return pair.head();
    }

    inline Pair cdr(Pair pair) noexcept
    {
        return pair.tail();
    }
}

namespace mll {

    // A quasy replacement for std::experimental::optional
    template <typename T> class Optional final {
    public:
        Optional() noexcept : engaged_{false}
        {
        }

        Optional(T t) noexcept : engaged_{true}
        {
            new (&value_) T(t);
        }

        Optional(Optional const& other) noexcept : engaged_{other.engaged_}
        {
            if (engaged_) {
                new (&value_) T(other.value_);
            }
        }

        ~Optional()
        {
            if (engaged_) {
                value_.~T();
            }
        }

        Optional& operator = (Optional const& rhs)
        {
            if (engaged_) {
                value_.~T();
            }

            engaged_ = rhs.engaged_;
            if (engaged_) {
                new (&value_) T(rhs.value_);
            }
        }

        operator bool() const noexcept
        {
            return engaged_;
        }

        T& operator *() const
        {
            assert(engaged_);
            return value_;
        }

        T* operator ->() const
        {
            assert(engaged_);
            return &value_;
        }

    private:
        bool engaged_;
        union {
            T value_;
        };
        bool operator == (Optional const&) = delete;
    };
}
