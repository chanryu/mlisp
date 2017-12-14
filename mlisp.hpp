#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

namespace mlisp {

    namespace detail {
        struct ObjectData;
        struct ConsData;
        struct ProcData;
        struct NumberData;
        struct StringData;
        struct SymbolData;
    }

    class Object;
    class Cons;
    class Proc;
    class Number;
    class String;
    class Symbol;

    struct Env;
    using EnvPtr = std::shared_ptr<Env>;
    using Func = std::function<Object(Cons, EnvPtr)>;

    Cons cons(Object, Cons) noexcept;
    Proc proc(Func) noexcept;
    Number number(double) noexcept;
    String string(std::string) noexcept;
    Symbol symbol(std::string) noexcept;

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

    class Object final {
    public:
        Object() noexcept;
        Object(Object const&) noexcept;
        Object(Cons const&) noexcept;
        Object(Number const&) noexcept;
        Object(String const&) noexcept;
        Object(Symbol const&) noexcept;
        Object(Proc const&) noexcept;

        Object& operator = (Object const&) noexcept;
        Object& operator = (Cons const&) noexcept;
        Object& operator = (Proc const&) noexcept;
        Object& operator = (Number const&) noexcept;
        Object& operator = (String const&) noexcept;
        Object& operator = (Symbol const&) noexcept;

        operator bool() const noexcept;

        void accept(ObjectVisitor&) const;

        Cons to_cons() const;
        Proc to_proc() const;
        Number to_number() const;
        String to_string() const;
        Symbol to_symbol() const;

        bool is_cons() const;
        bool is_proc() const;
        bool is_number() const;
        bool is_string() const;
        bool is_symbol() const;

    private:
        typedef detail::ObjectData Data;
        Object(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;

    private:
        bool operator == (Object const&) noexcept = delete;
    };

    class Cons final {
    public:
        Cons() noexcept;
        Cons(Cons const&) noexcept;

        Cons& operator = (Cons const&) noexcept;

        operator bool() const noexcept;

    private:
        friend class Object;
        friend struct detail::ConsData;
        friend Cons cons(Object, Cons) noexcept;
        friend Object car(Cons) noexcept;
        friend Cons cdr(Cons) noexcept;

        typedef detail::ConsData Data;
        Cons(std::shared_ptr<Data const>) noexcept;
        std::shared_ptr<Data const> data_;
    };

    class Proc final {
    public:
        Proc(Proc const&) noexcept;

        Object operator()(Cons, EnvPtr) const;

    private:
        friend class Object;
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
        friend class Object;
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
        friend class Object;
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
        friend class Object;
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
