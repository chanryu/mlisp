#include <cassert>
#include <sstream>

#include "mlisp.hpp"

namespace {
    using namespace mlisp;

    bool is_paren(char c) noexcept
    {
        return c == '(' || c == ')';
    }

    bool is_space(char c) noexcept
    {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }

    bool is_quote(char c) noexcept
    {
        return c == '\'';
    }

    void skip_whitespaces_and_comments(std::istream& istream)
    {
        auto in_comment = false;

        while (true) {
            auto c = istream.peek();
            if (c == EOF) {
                break;
            }

            if (in_comment) {
                istream.get();
                if (c == '\n') {
                    in_comment = false;
                }
            }
            else if (c == ';') {
                istream.get();
                in_comment = true;
            }
            else if (is_space(c)) {
                istream.get();
            }
            else {
                break;
            }
        }
    }

    std::string get_token(std::istream& istream) noexcept
    {
        std::string token;

        skip_whitespaces_and_comments(istream);

        char c;
        while (istream.get(c)) {
            if (is_space(c)) {
                assert(!token.empty());
                return token;
            }

            if (is_quote(c)) {
                token.push_back(c);
                return token;
            }

            if (is_paren(c)) {
                if (token.empty()) {
                    token.push_back(c);
                }
                else {
                    istream.unget();
                }
                return token;
            }
            token.push_back(c);
        }

        return token;
    }

    bool is_number(std::string const& token)
    {
        assert(!token.empty());

        char c = token[0];
        if (c == '-') {
            if (token.size() == 1) {
                return false;
            }
            c = token[1];
        }
        return c == '.' || (c >= '0' && c <= '9');
    }

    char const* const MLISP_BUILTIN_QUOTE = "~$~#~mlisp:built-in:quote~#~$~";
}

////////////////////////////////////////////////////////////////////////////////
// Object::Data

struct mlisp::Object::Data: std::enable_shared_from_this<Object::Data> {
    virtual ~Data() {}
    virtual void accept(ObjectVisitor&) const = 0;
};


////////////////////////////////////////////////////////////////////////////////
// Cons::Data

struct mlisp::Cons::Data: public mlisp::Object::Data {

    Data(Object h, Cons t) noexcept : head{h}, tail{t} { }

    void accept(ObjectVisitor& visitor) const override
    {
        visitor.visit(Cons{
            std::static_pointer_cast<Data const>(shared_from_this())
        });
    }

    Object const head;
    Cons const tail;
};

////////////////////////////////////////////////////////////////////////////////
// Proc::Data

struct mlisp::Proc::Data: public mlisp::Object::Data {

    explicit Data(Func f) noexcept : func{f} { }

    void accept(ObjectVisitor& visitor) const override
    {
        visitor.visit(Proc{
            std::static_pointer_cast<Data const>(shared_from_this())
        });
    }

    Func const func;
};

////////////////////////////////////////////////////////////////////////////////
// Number::Data

struct mlisp::Number::Data: public mlisp::Object::Data {

    explicit Data(double v) noexcept : value{v} { }

    void accept(ObjectVisitor& visitor) const override
    {
        visitor.visit(Number{
            std::static_pointer_cast<Data const>(shared_from_this())
        });
    }

    double const value;
};

////////////////////////////////////////////////////////////////////////////////
// String::Data

struct mlisp::String::Data: public mlisp::Object::Data {

    explicit Data(std::string t) noexcept : text{std::move(t)} { }

    void accept(ObjectVisitor& visitor) const override
    {
        visitor.visit(String{
            std::static_pointer_cast<Data const>(shared_from_this())
        });
    }

    std::string const text;
};

////////////////////////////////////////////////////////////////////////////////
// Symbol::Data

struct mlisp::Symbol::Data: public mlisp::Object::Data {

    explicit Data(std::string n) noexcept : name{std::move(n)} { }

    void accept(ObjectVisitor& visitor) const override
    {
        visitor.visit(Symbol{
            std::static_pointer_cast<Data const>(shared_from_this())
        });
    }

    std::string const name;
};

////////////////////////////////////////////////////////////////////////////////
// Object

mlisp::Object::Object() noexcept
{
}

mlisp::Object::Object(Object const& other) noexcept
    : data_{other.data_}
{
}

mlisp::Object::Object(Cons const& list) noexcept
    : data_{list.data_}
{
}

mlisp::Object::Object(Proc const& proc) noexcept
    : data_{proc.data_}
{
}

mlisp::Object::Object(Number const& number) noexcept
    : data_{number.data_}
{
}

mlisp::Object::Object(String const& string) noexcept
    : data_{string.data_}
{
}

mlisp::Object::Object(Symbol const& symbol) noexcept
    : data_{symbol.data_}
{
}

mlisp::Object::Object(std::shared_ptr<Data const> data) noexcept
    : data_{data}
{
}

mlisp::Object&
mlisp::Object::operator = (Object const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mlisp::Object&
mlisp::Object::operator = (Cons const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mlisp::Object&
mlisp::Object::operator = (Proc const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mlisp::Object&
mlisp::Object::operator = (Number const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mlisp::Object&
mlisp::Object::operator = (String const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mlisp::Object&
mlisp::Object::operator = (Symbol const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mlisp::Object::operator bool() const noexcept
{
    return !!data_;
}

void
mlisp::Object::accept(ObjectVisitor& visitor) const
{
    if (data_) {
        data_->accept(visitor);
    }
}

////////////////////////////////////////////////////////////////////////////////
// ConsCell

mlisp::Cons::Cons() noexcept
{
}

mlisp::Cons::Cons(Object head, Cons tail) noexcept
    : data_{ std::make_shared<Data>(head, tail) }
{
}

mlisp::Cons::Cons(Cons const& other) noexcept
    : data_{other.data_}
{
}

mlisp::Cons::Cons(std::shared_ptr<Data const> data) noexcept
    : data_{data}
{
}

mlisp::Cons&
mlisp::Cons::operator = (Cons const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mlisp::Cons::operator bool() const noexcept
{
    return !!data_;
}

////////////////////////////////////////////////////////////////////////////////
// Procedure

mlisp::Proc::Proc(Func func) noexcept
    : data_{ std::make_shared<Data>(func) }
{
}

mlisp::Proc::Proc(Proc const& other) noexcept
    : data_{other.data_}
{
}

mlisp::Proc::Proc(std::shared_ptr<Data const> data) noexcept
    : data_{data}
{
}

mlisp::Object
mlisp::Proc::operator()(Cons args, EnvPtr env) const
{
    if (data_->func) {
        return data_->func(args, env);
    }
    return {};  // nil
}

////////////////////////////////////////////////////////////////////////////////
// Number

mlisp::Number::Number(double value) noexcept
    : data_{ std::make_shared<Data>(value) }
{
}

mlisp::Number::Number(Number const& other) noexcept
    : data_{other.data_}
{
}

mlisp::Number::Number(std::shared_ptr<Data const> data) noexcept
    : data_{data}
{
}

double
mlisp::Number::value() const
{
    return data_->value;
}

////////////////////////////////////////////////////////////////////////////////
// String

mlisp::String::String(std::string text) noexcept
    : data_{ std::make_shared<Data>(std::move(text)) }
{
}

mlisp::String::String(String const& other) noexcept
    : data_{other.data_}
{
}

mlisp::String::String(std::shared_ptr<Data const> data) noexcept
    : data_{data}
{
}

std::string const&
mlisp::String::text() const
{
    return data_->text;
}

////////////////////////////////////////////////////////////////////////////////
// Symbol

mlisp::Symbol::Symbol(std::string name) noexcept
    : data_{ std::make_shared<Data>(std::move(name)) }
{
}

mlisp::Symbol::Symbol(Symbol const& other) noexcept
    : data_{other.data_}
{
}

mlisp::Symbol::Symbol(std::shared_ptr<Data const> data) noexcept
    : data_{data}
{
}

std::string const&
mlisp::Symbol::name() const
{
    return data_->name;
}


////////////////////////////////////////////////////////////////////////////////
// Parser

bool
mlisp::Parser::parse(std::istream& istream, Object& expr)
{
    while (true) {

        auto token = get_token(istream);

        if (token.empty()) {
            break;
        }

        if (token == "'") {
            stack_.push({ Context::Type::quote, {}, true });
            continue;
        }

        if (token == "(") {
            stack_.push({ Context::Type::paren, {}, true });
            continue;
        }

        Object obj;

        if (token == ")") {
            Cons list;
            while (true) {
                if (stack_.empty() ||
                    stack_.top().type == Context::Type::quote) {
                    throw ParseError{"Unexpected ')'"};
                }

                auto c = stack_.top();
                stack_.pop();
                if (c.head_empty) {
                    assert(c.type == Context::Type::paren);
                    assert(!list);
                }
                else {
                    list = Cons{ c.head, list };
                }
                if (c.type == Context::Type::paren) {
                    break;
                }
            }
            obj = list;
        }
        else if (is_number(token)) {
            obj = Number{ std::stod(token) };
        }
        else {
            obj = Symbol{ std::move(token) };
        }

        while (true) {
            if (stack_.empty()) {
                expr = obj;
                return true;
            }

            if (stack_.top().type == Context::Type::quote) {
                stack_.pop();
                obj = Cons{ Symbol{ MLISP_BUILTIN_QUOTE }, Cons{ obj, {} } };
                continue;
            }

            if (stack_.top().head_empty) {
                stack_.top().head = obj;
                stack_.top().head_empty = false;
            }
            else {
                stack_.push({ Context::Type::list, obj, false });
            }
            break;
        }
    }

    return false;
}

bool
mlisp::Parser::clean() const noexcept
{
    return stack_.empty();
}

////////////////////////////////////////////////////////////////////////////////
// Env

struct mlisp::Env {
    EnvPtr base;
    std::map<std::string, Object> vars;
};

EnvPtr
mlisp::make_env(EnvPtr base_env)
{
    auto env = std::make_shared<Env>();
    env->base = base_env;
    return env;
}

void
mlisp::set(EnvPtr env, std::string name, Object value)
{
    assert(env);
    env->vars[name] = value;
}

bool
mlisp::update(EnvPtr env, std::string const& name, Object value)
{
    assert(env);

    auto i = env->vars.find(name);
    if (i != env->vars.end()) {
        i->second = value;
        return true;
    }
    return false;
}

bool
mlisp::lookup(EnvPtr env, std::string const& name, Object& value)
{
    while (env) {
        auto i = env->vars.find(name);
        if (i != env->vars.end()) {
            value = i->second;
            return true;
        }
        env = env->base;
    }

    return false;
}


////////////////////////////////////////////////////////////////////////////////
// eval

namespace {
    class Evaluator: ObjectVisitor {
    public:
        explicit Evaluator(EnvPtr env) : env_(env)
        {
        }

        Object evaluate(Object expr)
        {
            if (expr) {
                expr.accept(*this);
            }
            else {
                result_ = Object{};
            }

            return result_;
        }

    private:
        void visit(Cons list) override
        {
            auto cmd = eval(car(list), env_);
            auto proc = to_proc(cmd);

            if (proc) {
                result_ = (*proc)(cdr(list), env_);
            } else {
                throw EvalError(std::to_string(cmd) + " is not a symbol.");
            }
        }

        void visit(Number num) override
        {
            result_ = num;
        }

        void visit(String str) override
        {
            result_ = str;
        }

        void visit(Symbol sym) override
        {
            if (sym.name() == MLISP_BUILTIN_QUOTE) {
                static auto quote_proc = make_proc([] (Cons args, EnvPtr) {
                    return car(args);
                });
                result_ = quote_proc;
            }
            else {
                Object value;
                if (lookup(env_, sym.name(), value)) {
                    result_ = value;
                } else {
                    throw EvalError("Unknown symbol: " + sym.name());
                }
            }
        }

        void visit(Proc proc) override
        {
            assert(false);
        }

    private:
        EnvPtr env_;
        Object result_;
    };
}

mlisp::Object
mlisp::eval(Object expr, EnvPtr env)
{
    return Evaluator(env).evaluate(expr);
}

////////////////////////////////////////////////////////////////////////////////
// Built-ins

mlisp::Object
mlisp::car(Cons list) noexcept
{
    return list.data_ ? list.data_->head : Object{};
}

mlisp::Cons
mlisp::cdr(Cons list) noexcept
{
    return list.data_ ? list.data_->tail : Cons{};
}

////////////////////////////////////////////////////////////////////////////////
// Printer

namespace {
    using namespace mlisp;

    class Printer: ObjectVisitor {
    public:
        Printer(std::ostream& ostream, bool is_head)
            : ostream_(ostream), is_head_(is_head) { }

        void print(Object const& obj)
        {
            if (obj) {
                obj.accept(*this);
            }
            else {
                ostream_ << "nil";
            }
        }

    private:
        void visit(Cons list) override
        {
            auto quoted = false;
            auto symbol = to_symbol(car(list));

            if (symbol && symbol->name() == MLISP_BUILTIN_QUOTE) {
                ostream_ << "'";
                quoted = true;
            }

            if (!quoted && is_head_) {
                ostream_ << '(';
            }
            if (!quoted) {
                auto head = car(list);
                Printer{ostream_, true}.print(head);
            }
            auto tail = cdr(list);
            if (tail) {
                if (!quoted) {
                    ostream_ << ' ';
                }
                Printer{ostream_, false}.print(tail);
            }
            if (!quoted && is_head_) {
                ostream_ << ')';
            }
        }

        void visit(Number num) override
        {
            ostream_ << num.value();
        }

        void visit(String str) override
        {
            ostream_ << str.text();
        }

        void visit(Symbol sym) override
        {
            if (sym.name() == MLISP_BUILTIN_QUOTE) {
                ostream_ << "quote";
            }
            else {
                ostream_ << sym.name();
            }
        }

        void visit(Proc proc) override
        {
            ostream_ << "<#procedure>";
        }

    private:
        std::ostream& ostream_;
        bool is_head_;
    };
}

std::ostream&
mlisp::operator << (std::ostream& ostream, mlisp::Object const& obj)
{
    Printer{ostream, true}.print(obj);
    return ostream;
}

std::ostream&
mlisp::operator << (std::ostream& ostream, mlisp::Cons const& ccl)
{
    return ostream << Object{ ccl };
}

std::string
std::to_string(mlisp::Object const& obj)
{
    return (ostringstream{} << obj).str();
}

////////////////////////////////////////////////////////////////////////////////
// Optional, to_xxx

mlisp::Optional<Cons>
mlisp::to_cons(Object obj) noexcept
{
    if (!obj.data_) {
        return Optional<Cons>{{}}; // nil
    }

    auto data = std::dynamic_pointer_cast<Cons::Data const>(obj.data_);
    if (data) {
        return { Cons{ data } };
    }

    return {};
}

mlisp::Optional<Proc>
mlisp::to_proc(Object obj) noexcept
{
    auto data = std::dynamic_pointer_cast<Proc::Data const>(obj.data_);
    if (data) {
        return { Proc{ data } };
    }

    return {};
}

mlisp::Optional<Number>
mlisp::to_number(Object obj) noexcept
{
    auto data = std::dynamic_pointer_cast<Number::Data const>(obj.data_);
    if (data) {
        return { Number{ data } };
    }

    return {};
}

mlisp::Optional<String>
mlisp::to_string(Object obj) noexcept
{
    auto data = std::dynamic_pointer_cast<String::Data const>(obj.data_);
    if (data) {
        return { String{ data } };
    }

    return {};
}

mlisp::Optional<Symbol>
mlisp::to_symbol(Object obj) noexcept
{
    auto data = std::dynamic_pointer_cast<Symbol::Data const>(obj.data_);
    if (data) {
        return { Symbol{ data } };
    }

    return {};
}
