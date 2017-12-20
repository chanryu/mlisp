#include <cassert>
#include <sstream>

#include "mll.hpp"

namespace {
    inline bool is_paren(char c)
    {
        return c == '(' || c == ')';
    }

    inline bool is_space(char c)
    {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }

    inline bool is_quote(char c)
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

            if (c == '"') {
                if (token.empty()) {
                    token.push_back(c);
                    auto escaped = false;
                    while (istream.get(c)) {
                        token.push_back(c);
                        if (escaped) {
                            escaped = false;
                        }
                        else if (c == '\\') {
                            escaped = true;
                        }
                        else if (c == '"') {
                            break;
                        }
                    }
                    return token;
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

    bool is_number_token(std::string const& token)
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

    bool is_string_token(std::string const& token)
    {
        assert(!token.empty());
        return token[0] == '"';
    }

    char const* const MLISP_BUILTIN_NIL = "nil";
    char const* const MLISP_BUILTIN_QUOTE = "quote";
}

////////////////////////////////////////////////////////////////////////////////
// Object::Data

struct mll::Node::Data: std::enable_shared_from_this<Node::Data> {
    virtual ~Data() {}
    virtual void accept(NodeVisitor&) = 0;
};


////////////////////////////////////////////////////////////////////////////////
// Pair::Data

struct mll::List::Data: public mll::Node::Data {

    Data(Node h, List t) noexcept : head{h}, tail{t} { }

    void accept(NodeVisitor& visitor) override
    {
        visitor.visit(List{
            std::static_pointer_cast<Data>(shared_from_this())
        });
    }

    Node const head;
    List const tail;
};

////////////////////////////////////////////////////////////////////////////////
// Proc::Data

struct mll::Proc::Data: public mll::Node::Data {

    explicit Data(Func f) noexcept : func{f} { }

    void accept(NodeVisitor& visitor) override
    {
        visitor.visit(Proc{
            std::static_pointer_cast<Data>(shared_from_this())
        });
    }

    Func const func;
};

////////////////////////////////////////////////////////////////////////////////
// Number::Data

struct mll::Number::Data: public mll::Node::Data {

    explicit Data(double v) noexcept : value{v} { }

    void accept(NodeVisitor& visitor) override
    {
        visitor.visit(Number{
            std::static_pointer_cast<Data>(shared_from_this())
        });
    }

    double const value;
};

////////////////////////////////////////////////////////////////////////////////
// String::Data

struct mll::String::Data: public mll::Node::Data {

    explicit Data(std::string t) noexcept : text{std::move(t)} { }

    void accept(NodeVisitor& visitor) override
    {
        visitor.visit(String{
            std::static_pointer_cast<Data>(shared_from_this())
        });
    }

    std::string const text;
};

////////////////////////////////////////////////////////////////////////////////
// Symbol::Data

struct mll::Symbol::Data: public mll::Node::Data {

    explicit Data(std::string n) noexcept : name{std::move(n)} { }

    void accept(NodeVisitor& visitor) override
    {
        visitor.visit(Symbol{
            std::static_pointer_cast<Data>(shared_from_this())
        });
    }

    std::string const name;
};

////////////////////////////////////////////////////////////////////////////////
// Object

mll::Node::Node() noexcept
{
}

mll::Node::Node(Node const& other) noexcept
    : data_{other.data_}
{
}

mll::Node::Node(List const& list) noexcept
    : data_{list.data_}
{
}

mll::Node::Node(Proc const& proc) noexcept
    : data_{proc.data_}
{
}

mll::Node::Node(Number const& number) noexcept
    : data_{number.data_}
{
}

mll::Node::Node(String const& string) noexcept
    : data_{string.data_}
{
}

mll::Node::Node(Symbol const& symbol) noexcept
    : data_{symbol.data_}
{
}

mll::Node&
mll::Node::operator = (Node const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mll::Node&
mll::Node::operator = (List const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mll::Node&
mll::Node::operator = (Proc const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mll::Node&
mll::Node::operator = (Number const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mll::Node&
mll::Node::operator = (String const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mll::Node&
mll::Node::operator = (Symbol const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

bool
mll::Node::operator == (Node const& rhs) noexcept
{
    return data_ == rhs.data_;
}

mll::Node::operator bool() const noexcept
{
    return !!data_;
}

void
mll::Node::accept(NodeVisitor& visitor)
{
    if (data_) {
        data_->accept(visitor);
    }
}

////////////////////////////////////////////////////////////////////////////////
// ConsCell

mll::List::List() noexcept
{
}

mll::List::List(Node head, List tail) noexcept
    : data_{ std::make_shared<Data>(head, tail) }
{
}

mll::List::List(List const& other) noexcept
    : data_{other.data_}
{
}

mll::List::List(std::shared_ptr<Data> data) noexcept
    : data_{data}
{
}

mll::List::operator bool() const noexcept
{
    return !!data_;
}

mll::Node
mll::List::head() const
{
    return data_ ? data_->head : Node{};
}

mll::List
mll::List::tail() const
{
    return data_ ? data_->tail : List{};
}


////////////////////////////////////////////////////////////////////////////////
// Procedure

mll::Proc::Proc(Func func) noexcept
    : data_{ std::make_shared<Data>(func) }
{
}

mll::Proc::Proc(Proc const& other) noexcept
    : data_{other.data_}
{
}

mll::Proc::Proc(std::shared_ptr<Data> data) noexcept
    : data_{data}
{
}

mll::Node
mll::Proc::operator()(List args, Env env) const
{
    if (data_->func) {
        return data_->func(args, env);
    }
    return {};  // nil
}

////////////////////////////////////////////////////////////////////////////////
// Number

mll::Number::Number(double value) noexcept
    : data_{ std::make_shared<Data>(value) }
{
}

mll::Number::Number(Number const& other) noexcept
    : data_{other.data_}
{
}

mll::Number::Number(std::shared_ptr<Data> data) noexcept
    : data_{data}
{
}

double
mll::Number::value() const
{
    return data_->value;
}

////////////////////////////////////////////////////////////////////////////////
// String

mll::String::String(std::string text) noexcept
    : data_{ std::make_shared<Data>(std::move(text)) }
{
}

mll::String::String(String const& other) noexcept
    : data_{other.data_}
{
}

mll::String::String(std::shared_ptr<Data> data) noexcept
    : data_{data}
{
}

std::string const&
mll::String::text() const
{
    return data_->text;
}

////////////////////////////////////////////////////////////////////////////////
// Symbol

mll::Symbol::Symbol(std::string name) noexcept
{
    thread_local std::map<std::string, std::shared_ptr<Data>> symbols;

    auto i = symbols.find(name);
    if (i == symbols.end()) {
        data_ = std::make_shared<Data>(std::move(name));
        symbols.insert(std::make_pair(data_->name, data_));
    } else {
        data_ = i->second;
    }
}

mll::Symbol::Symbol(Symbol const& other) noexcept
    : data_{other.data_}
{
}

mll::Symbol::Symbol(std::shared_ptr<Data> data) noexcept
    : data_{data}
{
}

std::string const&
mll::Symbol::name() const
{
    return data_->name;
}


////////////////////////////////////////////////////////////////////////////////
// Parser

mll::Optional<mll::Node>
mll::Parser::parse(std::istream& istream)
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
        else if (is_number_token(token)) {
            node = make_number(std::stod(token));
        }
        else if (is_string_token(token)) {
            node = make_string(translate(std::move(token)));
        }
        else {
            node = make_symbol(std::move(token));
        }

        while (true) {
            if (stack_.empty()) {
                return node;
            }

            if (stack_.top().type == Context::Type::quote) {
                stack_.pop();
                node = cons(make_symbol(MLISP_BUILTIN_QUOTE), cons(node, {}));
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
mll::Parser::clean() const noexcept
{
    return stack_.empty();
}

std::string
mll::Parser::translate(std::string token) const
{
    assert(token.length() >= 2);
    assert(token.front() == '"' && token.back() == '"');

    std::string text;

    auto escaped = false;
    for (size_t i = 1; i < token.size() - 1; ++i) {
        auto c = token[i];
        if (escaped) {
            escaped = false;
            // FIXME: more escaped chars
            if (c == 'n') {
                c = '\n';
            }
            text.push_back(c);
        }
        else if (c == '\\') {
            escaped = true;
        }
        else {
            text.push_back(c);
        }
    }

    return text;
}

////////////////////////////////////////////////////////////////////////////////
// Env

struct mll::Env::Data {
    std::shared_ptr<Data> base;
    std::map<std::string, Node> vars;
};

mll::Env::Env() : data_{ std::make_shared<Data>() }
{
}

mll::Env
mll::Env::derive_new() const
{
    Env new_env;
    new_env.data_->base = data_;
    return new_env;
}

void
mll::Env::set(std::string const& name, Node value)
{
    data_->vars[name] = value;
}

bool
mll::Env::update(std::string const& name, Node value)
{
    auto i = data_->vars.find(name);
    if (i != data_->vars.end()) {
        i->second = value;
        return true;
    }
    return false;
}

mll::Optional<mll::Node>
mll::Env::lookup(std::string const& name) const
{
    for (auto data = data_; data; data = data->base) {
        auto i = data->vars.find(name);
        if (i != data->vars.end()) {
            return i->second;
        }
    }
    return {};
}

mll::Optional<mll::Node>
mll::Env::shallow_lookup(std::string const& name) const
{
    auto i = data_->vars.find(name);
    if (i != data_->vars.end()) {
        return i->second;
    }
    return {};
}


////////////////////////////////////////////////////////////////////////////////
// eval

namespace mll {

    class Evaluator: NodeVisitor {
    public:
        explicit Evaluator(Env env) : env_(env) { }

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
            auto node = eval(car(list), env_);
            auto proc = to_proc(node);
            if (!proc) {
                throw EvalError(std::to_string(node) + " is not a proc.");
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
            if (sym.name() == MLISP_BUILTIN_NIL) {
                result_ = List{};
            }
            else if (sym.name() == MLISP_BUILTIN_QUOTE) {
                thread_local auto quote_proc = make_proc([](List args, Env) {
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

mll::Node
mll::eval(Node expr, Env env)
{
    return Evaluator(env).evaluate(expr);
}

////////////////////////////////////////////////////////////////////////////////
// Printer

namespace mll {
    class Printer: NodeVisitor {
    public:
        Printer(std::ostream& ostream, bool is_head)
            : ostream_(ostream), is_head_(is_head) { }

        void print(Node node)
        {
            if (node) {
                node.accept(*this);
            }
            else {
                visit(List{});
            }
        }

    private:
        void visit(List list) override
        {
            if (!list) {
                ostream_ << MLISP_BUILTIN_NIL;
                return;
            }

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
            ostream_ << sym.name();
        }

        void visit(Proc proc) override
        {
            ostream_ << "<#proc>";
        }

    private:
        std::ostream& ostream_;
        bool is_head_;
    };
}

std::ostream&
mll::operator << (std::ostream& ostream, mll::Node const& node)
{
    Printer{ostream, true}.print(node);
    return ostream;
}

std::ostream&
mll::operator << (std::ostream& ostream, mll::List const& ccl)
{
    return ostream << Node{ ccl };
}

std::string
std::to_string(mll::Node const& node)
{
    return (ostringstream{} << node).str();
}

////////////////////////////////////////////////////////////////////////////////
// Optional, to_xxx

mll::Optional<mll::List>
mll::to_list(Node node) noexcept
{
    if (!node.data_) {
        return Optional<List>{{}}; // nil
    }

    auto data = std::dynamic_pointer_cast<List::Data>(node.data_);
    if (data) {
        return { List{ data } };
    }

    return {};
}

mll::Optional<mll::Proc>
mll::to_proc(Node node) noexcept
{
    auto data = std::dynamic_pointer_cast<Proc::Data>(node.data_);
    if (data) {
        return { Proc{ data } };
    }

    return {};
}

mll::Optional<mll::Number>
mll::to_number(Node node) noexcept
{
    auto data = std::dynamic_pointer_cast<Number::Data>(node.data_);
    if (data) {
        return { Number{ data } };
    }

    return {};
}

mll::Optional<mll::String>
mll::to_string(Node node) noexcept
{
    auto data = std::dynamic_pointer_cast<String::Data>(node.data_);
    if (data) {
        return { String{ data } };
    }

    return {};
}

mll::Optional<mll::Symbol>
mll::to_symbol(Node node) noexcept
{
    auto data = std::dynamic_pointer_cast<Symbol::Data>(node.data_);
    if (data) {
        return { Symbol{ data } };
    }

    return {};
}