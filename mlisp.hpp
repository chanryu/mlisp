#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

namespace mlisp {

    class Object;
    class Cons;
    class Proc;
    class Number;
    class String;
    class Symbol;

    struct Env;
    using EnvPtr = std::shared_ptr<Env>;
    using Func = std::function<Object(Cons, EnvPtr)>;

    Object car(Cons) noexcept;
    Cons cdr(Cons) noexcept;

    class ObjectVisitor {
    public:
        virtual void visit(Cons) = 0;
        virtual void visit(Number) = 0;
        virtual void visit(String) = 0;
        virtual void visit(Symbol) = 0;
        virtual void visit(Proc) = 0;
    };

    template <typename T> class Optional;

    class Object final {
    public:
        Object() noexcept;
        Object(Object const&) noexcept;
        Object(Cons const&) noexcept;
        Object(Proc const&) noexcept;
        Object(Number const&) noexcept;
        Object(String const&) noexcept;
        Object(Symbol const&) noexcept;

        Object& operator = (Object const&) noexcept;
        Object& operator = (Cons const&) noexcept;
        Object& operator = (Proc const&) noexcept;
        Object& operator = (Number const&) noexcept;
        Object& operator = (String const&) noexcept;
        Object& operator = (Symbol const&) noexcept;

        operator bool() const noexcept;

        void accept(ObjectVisitor&) const;

        struct Data;
        friend Optional<Cons> to_cons(Object) noexcept;
        friend Optional<Proc> to_proc(Object) noexcept;
        friend Optional<Number> to_number(Object) noexcept;
        friend Optional<String> to_string(Object) noexcept;
        friend Optional<Symbol> to_symbol(Object) noexcept;

    private:
        Object(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;

    private:
        bool operator == (Object const&) noexcept = delete;
    };

    class Cons final {
    public:
        Cons() noexcept;
        Cons(Object head, Cons tail) noexcept;
        Cons(Cons const&) noexcept;

        Cons& operator = (Cons const&) noexcept;

        operator bool() const noexcept;

    public:
        struct Data;
        friend class Object;
        friend Object car(Cons) noexcept;
        friend Cons cdr(Cons) noexcept;
        friend Optional<Cons> to_cons(Object) noexcept;

    private:
        Cons(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;
    };

    class Proc final {
    public:
        explicit Proc(Func) noexcept;
        Proc(Proc const&) noexcept;

        Object operator()(Cons, EnvPtr) const;

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

    // parser

    class Parser {
    public:
        bool parse(std::istream&, Object&);
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
    // printing helper
    std::ostream& operator << (std::ostream& os, Object const&);
    std::ostream& operator << (std::ostream& os, Cons const&);
}

namespace std {
    string to_string(mlisp::Object const&);
}

namespace mlisp {
    template <typename T>
    class Optional {
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

    Optional<Cons> to_cons(Object) noexcept;
    Optional<Proc> to_proc(Object) noexcept;
    Optional<Number> to_number(Object) noexcept;
    Optional<String> to_string(Object) noexcept;
    Optional<Symbol> to_symbol(Object) noexcept;
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
        using UniqueRuntimeError::UniqueRuntimeError;
    };

    struct EvalError: detail::UniqueRuntimeError<1> {
        using UniqueRuntimeError::UniqueRuntimeError;
    };
}

namespace mlisp {

    inline Cons make_cons(Object head, Cons tail) noexcept
    {
        return Cons{ head, tail };
    }

    inline Proc make_proc(Func func) noexcept
    {
        return Proc{ func };
    }
}
