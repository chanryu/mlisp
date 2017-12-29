#ifndef __MLL_HPP__
#define __MLL_HPP__

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
    class List;
    class Proc;
    class Number;
    class String;
    class Symbol;

    class Env;
    using Func = std::function<Node(List, std::shared_ptr<Env>)>;

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

        bool operator == (Node const&);
        operator bool() const;

        void accept(NodeVisitor&);

        struct Data;
        std::shared_ptr<Data> const& data() const;

    private:
        std::shared_ptr<Data> data_;
    };

    class List final {
    public:
        List() = default;

        List(Node head, List tail);
        List(List const&);

        operator bool() const;

        Node head() const;
        List tail() const;

    public:
        struct Data;
        friend class Node;
        friend Optional<List> to_list(Node const&);

    private:
        List(std::shared_ptr<Data>);
        std::shared_ptr<Data> data_;
    };

    class Proc final {
    public:
        explicit Proc(Func);
        Proc(Proc const&);

        Node call(List, std::shared_ptr<Env>) const;

    public:
        struct Data;
        friend class Node;
        friend Optional<Proc> to_proc(Node const&);

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
        friend Optional<Number> to_number(Node const&);

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
        friend Optional<String> to_string(Node const&);

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
        friend Optional<Symbol> to_symbol(Node const&);

    private:
        Symbol(std::shared_ptr<Data>);
        std::shared_ptr<Data> data_;
    };

    // casting functions
    Optional<List>   to_list(Node const&);
    Optional<Proc>   to_proc(Node const&);
    Optional<Number> to_number(Node const&);
    Optional<String> to_string(Node const&);
    Optional<Symbol> to_symbol(Node const&);
}

namespace mll {

    // Parser

    class Parser {
    public:
        Optional<Node> parse(std::istream&);
        bool clean() const;

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

    class Env: public std::enable_shared_from_this<Env> {
    public:
        static std::shared_ptr<Env> create();
        std::shared_ptr<Env> derive_new() const;

        void set(std::string const&, Node const&);
        bool update(std::string const&, Node const&);
        Optional<Node> lookup(std::string const&) const;
        Optional<Node> shallow_lookup(std::string const&) const;

    private:
        Env() = default;
        std::shared_ptr<Env const> base_;
        std::map<std::string, Node> vars_;
    };

    Node eval(Node expr, std::shared_ptr<Env> env); // throws EvalError
}

namespace mll {
    class BasicPrinter: public NodeVisitor {
    public:
        explicit BasicPrinter(std::ostream& ostream);

        void print(Node node);

        void visit(List list) override;
        void visit(Proc proc) override;
        void visit(Number num) override;
        void visit(String str) override;
        void visit(Symbol sym) override;

    private:
        BasicPrinter(std::ostream& ostream, bool is_head);

        std::ostream& ostream_;
        bool is_head_;
    };

    std::ostream& operator << (std::ostream& os, Node const&);
}

namespace std {
    string to_string(mll::Node const&);
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
}

namespace mll {

    // A quasy replacement for std::experimental::optional
    template <typename T> class Optional final {
    public:
        Optional() : engaged_{false}
        {
        }

        Optional(T t) : engaged_{true}
        {
            new (&value_) T(t);
        }

        Optional(Optional const& other) : engaged_{other.engaged_}
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

        operator bool() const
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

#endif // __MLL_HPP__
