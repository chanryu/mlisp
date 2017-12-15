#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

namespace mlisp {

    class Object;
    class Pair;
    class Proc;
    class Number;
    class String;
    class Symbol;

    struct Env;
    using EnvPtr = std::shared_ptr<Env>;
    using Func = std::function<Object(Pair, EnvPtr)>;

    class ObjectVisitor {
    public:
        virtual void visit(Pair) = 0;
        virtual void visit(Proc) = 0;
        virtual void visit(Number) = 0;
        virtual void visit(String) = 0;
        virtual void visit(Symbol) = 0;
    };

    template <typename T> class Optional;

    class Object final {
    public:
        Object() noexcept;
        Object(Object const&) noexcept;
        Object(Pair const&) noexcept;
        Object(Proc const&) noexcept;
        Object(Number const&) noexcept;
        Object(String const&) noexcept;
        Object(Symbol const&) noexcept;

        Object& operator = (Object const&) noexcept;
        Object& operator = (Pair const&) noexcept;
        Object& operator = (Proc const&) noexcept;
        Object& operator = (Number const&) noexcept;
        Object& operator = (String const&) noexcept;
        Object& operator = (Symbol const&) noexcept;

        operator bool() const noexcept;

        void accept(ObjectVisitor&) const;

        struct Data;
        friend Optional<Pair> to_pair(Object) noexcept;
        friend Optional<Proc> to_proc(Object) noexcept;
        friend Optional<Number> to_number(Object) noexcept;
        friend Optional<String> to_string(Object) noexcept;
        friend Optional<Symbol> to_symbol(Object) noexcept;

    private:
        std::shared_ptr<Data const> data_;

    private:
        bool operator == (Object const&) noexcept = delete;
    };

    class Pair final {
    public:
        Pair() noexcept;
        Pair(Object head, Pair tail) noexcept;
        Pair(Pair const&) noexcept;

        operator bool() const noexcept;

        Object head() const;
        Pair tail() const;

    public:
        struct Data;
        friend class Object;
        friend Object car(Pair) noexcept;
        friend Pair cdr(Pair) noexcept;
        friend Optional<Pair> to_pair(Object) noexcept;

    private:
        Pair(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;
    };

    class Proc final {
    public:
        explicit Proc(Func) noexcept;
        Proc(Proc const&) noexcept;

        Object operator()(Pair, EnvPtr) const;

    public:
        struct Data;
        friend class Object;
        friend Optional<Proc> to_proc(Object) noexcept;

    private:
        Proc(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;
    };

    class Number final {
    public:
        explicit Number(double) noexcept;
        Number(Number const&) noexcept;

        double value() const;

    public:
        struct Data;
        friend class Object;
        friend Optional<Number> to_number(Object) noexcept;

    private:
        Number(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;
    };

    class String final {
    public:
        explicit String(std::string) noexcept;
        String(String const&) noexcept;

        std::string const& text() const;

    public:
        struct Data;
        friend class Object;
        friend Optional<String> to_string(Object) noexcept;

    private:
        String(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;
    };

    class Symbol final {
    public:
        explicit Symbol(std::string) noexcept;
        Symbol(Symbol const&) noexcept;

        std::string const& name() const;

    public:
        struct Data;
        friend class Object;
        friend Optional<Symbol> to_symbol(Object) noexcept;

    private:
        Symbol(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;
    };
}

namespace mlisp {

    // Parser

    class Parser {
    public:
        Optional<Object> parse(std::istream&);
        bool clean() const noexcept;

    private:
        struct Context {
            enum class Type { quote, paren, list };
            Type type;
            Object head;
            bool head_empty;
        };
        std::stack<Context> stack_;
    };
}

namespace mlisp {

    // Env & eval

    EnvPtr make_env(EnvPtr base_env);
    void set(EnvPtr, std::string, Object);
    bool update(EnvPtr, std::string const&, Object);
    bool lookup(EnvPtr, std::string const&, Object&);

    Object eval(Object expr, EnvPtr env); // throws EvalError
}

namespace mlisp {

    // Overloaded ostream << operators

    std::ostream& operator << (std::ostream& os, Object const&);
    std::ostream& operator << (std::ostream& os, Pair const&);
}

namespace std {
    string to_string(mlisp::Object const&);
}

namespace mlisp {

    // Casting functions

    Optional<Pair> to_pair(Object) noexcept;
    Optional<Proc> to_proc(Object) noexcept;
    Optional<Number> to_number(Object) noexcept;
    Optional<String> to_string(Object) noexcept;
    Optional<Symbol> to_symbol(Object) noexcept;
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

    inline Pair make_pair(Object head, Pair tail) noexcept
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

    inline Pair cons(Object head, Pair tail) noexcept
    {
        return make_pair(head, tail);
    }

    inline Object car(Pair pair) noexcept
    {
        return pair.head();
    }

    inline Pair cdr(Pair pair) noexcept
    {
        return pair.tail();
    }
}

namespace mlisp {

    // Optional template

    template <typename T> class Optional {
    public:
        Optional() noexcept {}
        Optional(T t) noexcept : t_ { std::make_shared<T>(t) } {}

        operator bool() const noexcept
        {
            return !!t_;
        }

        T& operator *() const
        {
            return *t_;
        }

        T* operator ->() const
        {
            return t_.get();
        }

    private:
        std::shared_ptr<T> t_;
    };
}
