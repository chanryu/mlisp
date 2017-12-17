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
// Pair::Data

struct mlisp::Pair::Data: public mlisp::Object::Data {

    Data(Object h, Pair t) noexcept : head{h}, tail{t} { }

    void accept(ObjectVisitor& visitor) const override
    {
        visitor.visit(Pair{
            std::static_pointer_cast<Data const>(shared_from_this())
        });
    }

    Object const head;
    Pair const tail;
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

mlisp::Object::Object(Pair const& list) noexcept
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

mlisp::Object&
mlisp::Object::operator = (Object const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mlisp::Object&
mlisp::Object::operator = (Pair const& rhs) noexcept
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

mlisp::Pair::Pair() noexcept
{
}

mlisp::Pair::Pair(Object head, Pair tail) noexcept
    : data_{ std::make_shared<Data>(head, tail) }
{
}

mlisp::Pair::Pair(Pair const& other) noexcept
    : data_{other.data_}
{
}

mlisp::Pair::Pair(std::shared_ptr<Data const> data) noexcept
    : data_{data}
{
}

mlisp::Pair::operator bool() const noexcept
{
    return !!data_;
}

mlisp::Object
mlisp::Pair::head() const
{
    return data_ ? data_->head : Object{};
}

mlisp::Pair
mlisp::Pair::tail() const
{
    return data_ ? data_->tail : Pair{};
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
mlisp::Proc::operator()(Pair args, Env env) const
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

mlisp::Optional<Object>
mlisp::Parser::parse(std::istream& istream)
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
            Pair list;
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
                    list = cons(c.head, list);
                }
                if (c.type == Context::Type::paren) {
                    break;
                }
            }
            obj = list;
        }
        else if (is_number(token)) {
            obj = make_number(std::stod(token));
        }
        else {
            obj = Symbol{ std::move(token) };
        }

        while (true) {
            if (stack_.empty()) {
                return obj;
            }

            if (stack_.top().type == Context::Type::quote) {
                stack_.pop();
                obj = cons(Symbol{ MLISP_BUILTIN_QUOTE }, cons(obj, {}));
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

    return {};
}

bool
mlisp::Parser::clean() const noexcept
{
    return stack_.empty();
}

////////////////////////////////////////////////////////////////////////////////
// Env

struct mlisp::Env::Data {
    std::shared_ptr<Data> base;
    std::map<std::string, Object> vars;
};

mlisp::Env::Env() : data_{ std::make_shared<Data>() }
{
}

Env
mlisp::Env::derive_new() const
{
    Env new_env;
    new_env.data_->base = data_;
    return new_env;
}

void
mlisp::Env::set(std::string const& name, Object value)
{
    data_->vars[name] = value;
}

bool
mlisp::Env::update(std::string const& name, Object value)
{
    auto i = data_->vars.find(name);
    if (i != data_->vars.end()) {
        i->second = value;
        return true;
    }
    return false;
}

mlisp::Optional<Object>
mlisp::Env::lookup(std::string const& name) const
{
    Optional<Object> value;
    for (auto data = data_; data; data = data->base) {
        auto i = data->vars.find(name);
        if (i != data->vars.end()) {
            value = i->second;
            break;
        }
    }
    return value;
}


////////////////////////////////////////////////////////////////////////////////
// eval

namespace {
    class Evaluator: ObjectVisitor {
    public:
        explicit Evaluator(Env env) : env_(env)
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
        void visit(Pair pair) override
        {
            auto cmd = eval(car(pair), env_);
            auto proc = to_proc(cmd);
            if (!proc) {
                throw EvalError(std::to_string(cmd) + " is not a proc.");
            }

            result_ = (*proc)(cdr(pair), env_);
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
                thread_local auto quote_proc = make_proc([] (Pair args, Env) {
                    return car(args);
                });
                result_ = quote_proc;
            }
            else {
                auto value = env_.lookup(sym.name());
                if (!value) {
                    throw EvalError("Unknown symbol: " + sym.name());
                }
                result_ = *value;
            }
        }

        void visit(Proc proc) override
        {
            assert(false);
        }

    private:
        Env env_;
        Object result_;
    };
}

mlisp::Object
mlisp::eval(Object expr, Env env)
{
    return Evaluator(env).evaluate(expr);
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
        void visit(Pair pair) override
        {
            auto quoted = false;
            auto symbol = to_symbol(car(pair));

            if (symbol && symbol->name() == MLISP_BUILTIN_QUOTE) {
                ostream_ << "'";
                quoted = true;
            }

            if (!quoted && is_head_) {
                ostream_ << '(';
            }
            if (!quoted) {
                auto head = car(pair);
                Printer{ostream_, true}.print(head);
            }
            auto tail = cdr(pair);
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
mlisp::operator << (std::ostream& ostream, mlisp::Pair const& ccl)
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

mlisp::Optional<Pair>
mlisp::to_pair(Object obj) noexcept
{
    if (!obj.data_) {
        return Optional<Pair>{{}}; // nil
    }

    auto data = std::dynamic_pointer_cast<Pair::Data const>(obj.data_);
    if (data) {
        return { Pair{ data } };
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
