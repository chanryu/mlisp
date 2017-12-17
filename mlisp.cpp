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

struct mlisp::Node::Data: std::enable_shared_from_this<Node::Data> {
    virtual ~Data() {}
    virtual void accept(ObjectVisitor&) const = 0;
};


////////////////////////////////////////////////////////////////////////////////
// Pair::Data

struct mlisp::List::Data: public mlisp::Node::Data {

    Data(Node h, List t) noexcept : head{h}, tail{t} { }

    void accept(ObjectVisitor& visitor) const override
    {
        visitor.visit(List{
            std::static_pointer_cast<Data const>(shared_from_this())
        });
    }

    Node const head;
    List const tail;
};

////////////////////////////////////////////////////////////////////////////////
// Proc::Data

struct mlisp::Proc::Data: public mlisp::Node::Data {

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

struct mlisp::Number::Data: public mlisp::Node::Data {

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

struct mlisp::String::Data: public mlisp::Node::Data {

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

struct mlisp::Symbol::Data: public mlisp::Node::Data {

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

mlisp::Node::Node() noexcept
{
}

mlisp::Node::Node(Node const& other) noexcept
    : data_{other.data_}
{
}

mlisp::Node::Node(List const& list) noexcept
    : data_{list.data_}
{
}

mlisp::Node::Node(Proc const& proc) noexcept
    : data_{proc.data_}
{
}

mlisp::Node::Node(Number const& number) noexcept
    : data_{number.data_}
{
}

mlisp::Node::Node(String const& string) noexcept
    : data_{string.data_}
{
}

mlisp::Node::Node(Symbol const& symbol) noexcept
    : data_{symbol.data_}
{
}

mlisp::Node&
mlisp::Node::operator = (Node const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mlisp::Node&
mlisp::Node::operator = (List const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mlisp::Node&
mlisp::Node::operator = (Proc const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mlisp::Node&
mlisp::Node::operator = (Number const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mlisp::Node&
mlisp::Node::operator = (String const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mlisp::Node&
mlisp::Node::operator = (Symbol const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mlisp::Node::operator bool() const noexcept
{
    return !!data_;
}

void
mlisp::Node::accept(ObjectVisitor& visitor) const
{
    if (data_) {
        data_->accept(visitor);
    }
}

////////////////////////////////////////////////////////////////////////////////
// ConsCell

mlisp::List::List() noexcept
{
}

mlisp::List::List(Node head, List tail) noexcept
    : data_{ std::make_shared<Data>(head, tail) }
{
}

mlisp::List::List(List const& other) noexcept
    : data_{other.data_}
{
}

mlisp::List::List(std::shared_ptr<Data const> data) noexcept
    : data_{data}
{
}

mlisp::List::operator bool() const noexcept
{
    return !!data_;
}

mlisp::Node
mlisp::List::head() const
{
    return data_ ? data_->head : Node{};
}

mlisp::List
mlisp::List::tail() const
{
    return data_ ? data_->tail : List{};
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

mlisp::Node
mlisp::Proc::operator()(List args, Env env) const
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
{
    thread_local std::map<std::string, std::shared_ptr<Data const>> symbols;

    auto i = symbols.find(name);
    if (i == symbols.end()) {
        data_ = std::make_shared<Data const>(std::move(name));
        symbols.insert(std::make_pair(data_->name, data_));
    } else {
        data_ = i->second;
    }
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

mlisp::Optional<Node>
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

        Node node;

        if (token == ")") {
            List list;
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
            node = list;
        }
        else if (is_number(token)) {
            node = make_number(std::stod(token));
        }
        else {
            node = Symbol{ std::move(token) };
        }

        while (true) {
            if (stack_.empty()) {
                return node;
            }

            if (stack_.top().type == Context::Type::quote) {
                stack_.pop();
                node = cons(Symbol{ MLISP_BUILTIN_QUOTE }, cons(node, {}));
                continue;
            }

            if (stack_.top().head_empty) {
                stack_.top().head = node;
                stack_.top().head_empty = false;
            }
            else {
                stack_.push({ Context::Type::list, node, false });
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
    std::map<std::string, Node> vars;
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
mlisp::Env::set(std::string const& name, Node value)
{
    data_->vars[name] = value;
}

bool
mlisp::Env::update(std::string const& name, Node value)
{
    auto i = data_->vars.find(name);
    if (i != data_->vars.end()) {
        i->second = value;
        return true;
    }
    return false;
}

mlisp::Optional<Node>
mlisp::Env::lookup(std::string const& name) const
{
    Optional<Node> value;
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

        Node evaluate(Node expr)
        {
            if (expr) {
                expr.accept(*this);
            }
            else {
                result_ = Node{};
            }

            return result_;
        }

    private:
        void visit(List list) override
        {
            auto cmd = eval(car(list), env_);
            auto proc = to_proc(cmd);
            if (!proc) {
                throw EvalError(std::to_string(cmd) + " is not a proc.");
            }

            result_ = (*proc)(cdr(list), env_);
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
                thread_local auto quote_proc = make_proc([] (List args, Env) {
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
        Node result_;
    };
}

mlisp::Node
mlisp::eval(Node expr, Env env)
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

        void print(Node const& node)
        {
            if (node) {
                node.accept(*this);
            }
            else {
                ostream_ << "nil";
            }
        }

    private:
        void visit(List list) override
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
mlisp::operator << (std::ostream& ostream, mlisp::Node const& node)
{
    Printer{ostream, true}.print(node);
    return ostream;
}

std::ostream&
mlisp::operator << (std::ostream& ostream, mlisp::List const& ccl)
{
    return ostream << Node{ ccl };
}

std::string
std::to_string(mlisp::Node const& node)
{
    return (ostringstream{} << node).str();
}

////////////////////////////////////////////////////////////////////////////////
// Optional, to_xxx

mlisp::Optional<List>
mlisp::to_list(Node node) noexcept
{
    if (!node.data_) {
        return Optional<List>{{}}; // nil
    }

    auto data = std::dynamic_pointer_cast<List::Data const>(node.data_);
    if (data) {
        return { List{ data } };
    }

    return {};
}

mlisp::Optional<Proc>
mlisp::to_proc(Node node) noexcept
{
    auto data = std::dynamic_pointer_cast<Proc::Data const>(node.data_);
    if (data) {
        return { Proc{ data } };
    }

    return {};
}

mlisp::Optional<Number>
mlisp::to_number(Node node) noexcept
{
    auto data = std::dynamic_pointer_cast<Number::Data const>(node.data_);
    if (data) {
        return { Number{ data } };
    }

    return {};
}

mlisp::Optional<String>
mlisp::to_string(Node node) noexcept
{
    auto data = std::dynamic_pointer_cast<String::Data const>(node.data_);
    if (data) {
        return { String{ data } };
    }

    return {};
}

mlisp::Optional<Symbol>
mlisp::to_symbol(Node node) noexcept
{
    auto data = std::dynamic_pointer_cast<Symbol::Data const>(node.data_);
    if (data) {
        return { Symbol{ data } };
    }

    return {};
}
