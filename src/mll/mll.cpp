#include "mll.hpp"

#include <array>
#include <cassert>
#include <sstream>

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

    bool is_number_token(std::string const& token)
    {
        assert(!token.empty());

        char const* s = token.c_str();
        if (*s == '-') s++;
        if (*s == '.') s++;
        if (*s < '0' || *s > '9') return false;

        size_t len;
        std::stod(token.c_str(), &len);
        return token.length() == len;
    }

    bool is_string_token(std::string const& token)
    {
        assert(!token.empty());
        return token[0] == '"';
    }

    std::string quote_text(std::string const& text)
    {
        std::string quoted_text;
        quoted_text.reserve(static_cast<size_t>(text.size() * 1.5) + 2);
        quoted_text.push_back('\"');
        for (auto c: text) {
            if (c == '\"') {
                quoted_text.push_back('\\');
            }
            quoted_text.push_back(c);
        }
        quoted_text.push_back('\"');
        return quoted_text;
    }

    static mll::List const nil;

    char const* const MLL_QUOTE = "quote";
}

////////////////////////////////////////////////////////////////////////////////
// Node::Data

struct mll::Node::Data: std::enable_shared_from_this<Node::Data> {
    virtual ~Data() {}
    virtual void accept(NodeVisitor&) = 0;
};

////////////////////////////////////////////////////////////////////////////////
// List::Data

struct mll::List::Data: mll::Node::Data {

    Data(Node h, List t) : head{h}, tail{t} { }

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

struct mll::Proc::Data: mll::Node::Data {

    explicit Data(Func f) : func{std::move(f)} { }

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

struct mll::Number::Data: mll::Node::Data {

    explicit Data(double v) : value{v} { }

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

struct mll::String::Data: mll::Node::Data {

    explicit Data(std::string t) : text{std::move(t)} { }

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

struct mll::Symbol::Data: mll::Node::Data {

    explicit Data(std::string n) : name{std::move(n)} { }

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

mll::Node::Node(Node const& other)
    : data_{other.data_}
{
}

mll::Node::Node(List const& list)
    : data_{list.data_}
{
}

mll::Node::Node(Proc const& proc)
    : data_{proc.data_}
{
}

mll::Node::Node(Number const& number)
    : data_{number.data_}
{
}

mll::Node::Node(String const& string)
    : data_{string.data_}
{
}

mll::Node::Node(Symbol const& symbol)
    : data_{symbol.data_}
{
}

mll::Node&
mll::Node::operator = (Node const& rhs)
{
    data_ = rhs.data_;
    return *this;
}

mll::Node&
mll::Node::operator = (List const& rhs)
{
    data_ = rhs.data_;
    return *this;
}

mll::Node&
mll::Node::operator = (Proc const& rhs)
{
    data_ = rhs.data_;
    return *this;
}

mll::Node&
mll::Node::operator = (Number const& rhs)
{
    data_ = rhs.data_;
    return *this;
}

mll::Node&
mll::Node::operator = (String const& rhs)
{
    data_ = rhs.data_;
    return *this;
}

mll::Node&
mll::Node::operator = (Symbol const& rhs)
{
    data_ = rhs.data_;
    return *this;
}

void
mll::Node::accept(NodeVisitor& visitor)
{
    if (data_) {
        data_->accept(visitor);
    } else {
        visitor.visit(nil);
    }
}

std::shared_ptr<mll::Node::Data> const&
mll::Node::data() const
{
    return data_;
}

////////////////////////////////////////////////////////////////////////////////
// List

mll::List::List(Node head, List tail)
    : data_{ std::make_shared<Data>(head, tail) }
{
}

mll::List::List(List const& other)
    : data_{other.data_}
{
}

mll::List::List(std::shared_ptr<Data> data)
    : data_{data}
{
}

bool
mll::List::empty() const
{
    return !data_;
}

mll::Node
mll::List::head() const
{
    return data_ ? data_->head : nil;
}

mll::List
mll::List::tail() const
{
    return data_ ? data_->tail : nil;
}


////////////////////////////////////////////////////////////////////////////////
// Proc

mll::Proc::Proc(Func func)
    : data_{ std::make_shared<Data>(func) }
{
}

mll::Proc::Proc(Proc const& other)
    : data_{other.data_}
{
}

mll::Proc::Proc(std::shared_ptr<Data> data)
    : data_{data}
{
}

mll::Node
mll::Proc::call(List args, std::shared_ptr<Env> env) const
{
    if (data_->func) {
        return data_->func(args, env);
    }
    return nil;
}

////////////////////////////////////////////////////////////////////////////////
// Number

mll::Number::Number(double value)
    : data_{ std::make_shared<Data>(value) }
{
}

mll::Number::Number(Number const& other)
    : data_{other.data_}
{
}

mll::Number::Number(std::shared_ptr<Data> data)
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

mll::String::String(std::string text)
    : data_{ std::make_shared<Data>(std::move(text)) }
{
}

mll::String::String(String const& other)
    : data_{other.data_}
{
}

mll::String::String(std::shared_ptr<Data> data)
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

mll::Symbol::Symbol(std::string name)
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

mll::Symbol::Symbol(Symbol const& other)
    : data_{other.data_}
{
}

mll::Symbol::Symbol(std::shared_ptr<Data> data)
    : data_{data}
{
}

std::string const&
mll::Symbol::name() const
{
    return data_->name;
}

////////////////////////////////////////////////////////////////////////////////
// Casting functions

std::optional<mll::List>
mll::to_list(Node const& node)
{
    if (!node.data()) {
        return nil;
    }

    auto data = std::dynamic_pointer_cast<List::Data>(node.data());
    if (data) {
        return { List{ data } };
    }

    return {};
}

std::optional<mll::Proc>
mll::to_proc(Node const& node)
{
    auto data = std::dynamic_pointer_cast<Proc::Data>(node.data());
    if (data) {
        return { Proc{ data } };
    }

    return {};
}

std::optional<mll::Number>
mll::to_number(Node const& node)
{
    auto data = std::dynamic_pointer_cast<Number::Data>(node.data());
    if (data) {
        return { Number{ data } };
    }

    return {};
}

std::optional<mll::String>
mll::to_string(Node const& node)
{
    auto data = std::dynamic_pointer_cast<String::Data>(node.data());
    if (data) {
        return { String{ data } };
    }

    return {};
}

std::optional<mll::Symbol>
mll::to_symbol(Node const& node)
{
    auto data = std::dynamic_pointer_cast<Symbol::Data>(node.data());
    if (data) {
        return { Symbol{ data } };
    }

    return {};
}

////////////////////////////////////////////////////////////////////////////////
// Parser

std::optional<mll::Node>
mll::Parser::parse(std::istream& istream)
{
    while (true) {

        if (!get_token(istream))
            break;

        assert(!token_.empty());

        if (token_ == "'") {
            token_.clear();
            stack_.push({ Context::Type::quote, {}, true });
            continue;
        }

        if (token_ == "(") {
            token_.clear();
            stack_.push({ Context::Type::paren, {}, true });
            continue;
        }

        Node node;

        if (token_ == ")") {
            List list;
            while (true) {
                if (stack_.empty() ||
                    stack_.top().type == Context::Type::quote) {
                    token_.clear();
                    throw ParseError{"redundant ')'"};
                }

                auto c = stack_.top();
                stack_.pop();
                if (c.head_empty) {
                    assert(c.type == Context::Type::paren);
                    assert(list.empty());
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
        else {
            node = make_node(std::move(token_));
        }

        token_.clear();

        while (true) {
            if (stack_.empty()) {
                return node;
            }

            if (stack_.top().type == Context::Type::quote) {
                stack_.pop();
                node = cons(make_symbol(MLL_QUOTE), cons(node, nil));
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
mll::Parser::clean() const
{
    return stack_.empty();
}

mll::Node
mll::Parser::make_node(std::string token)
{
    if (is_number_token(token)) {
        return make_number(std::stod(token));
    }

    if (is_string_token(token)) {
        assert(token.length() >= 2);
        assert(token.front() == '"' && token.back() == '"');

        static const std::array<char, 128> esctbl = {{
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, '\"', 0x00, 0x00, 0x00, 0x00, '\'',
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\?',
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, '\\', 0x00, 0x00, 0x00,
            0x00, '\a', '\b', 0x00, 0x00, 0x00, '\f', 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\n', 0x00,
            0x00, 0x00, '\r', 0x00, '\t', 0x00, '\v', 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        }};

        std::string text;

        auto escaped = false;
        for (size_t i = 1; i < token.size() - 1; ++i) {
            auto c = token[i];
            if (escaped) {
                escaped = false;

                if (c >= 0 && static_cast<size_t>(c) < esctbl.size() && static_cast<bool>(esctbl[c])) {
                    text.push_back(esctbl[c]);
                }
                else {
                    text.push_back('\\');
                    text.push_back(c);
                }
            }
            else if (c == '\\') {
                escaped = true;
            }
            else {
                text.push_back(c);
            }
        }

        return make_string(std::move(text));
    }

    return make_symbol(std::move(token));
}

bool
mll::Parser::get_token(std::istream& istream)
{
    auto read_string_token = [this, &istream]() {
        assert(!token_.empty());
        assert(token_[0] == '"');
        char c;
        while (istream.get(c)) {
            token_.push_back(c);
            if (token_escaped_) {
                token_escaped_ = false;
            }
            else if (c == '\\') {
                token_escaped_ = true;
            }
            else if (c == '"') {
                return true;
            }
        }
        return false;
    };

    if (!token_.empty()) {
        // we have unfinished string token
        return read_string_token();
    }

    skip_whitespaces_and_comments(istream);

    char c;
    while (istream.get(c)) {
        if (is_space(c)) {
            assert(!token_.empty());
            return true;
        }

        if (is_quote(c)) {
            token_.push_back(c);
            return true;
        }

        if (is_paren(c)) {
            if (token_.empty()) {
                token_.push_back(c);
            }
            else {
                istream.unget();
            }
            return true;
        }

        if (c == '"') {
            if (token_.empty()) {
                token_.push_back(c);
                token_escaped_ = false;
                return read_string_token();
            }

            istream.unget();
            return true;
        }

        token_.push_back(c);
    }

    return !token_.empty();
}

////////////////////////////////////////////////////////////////////////////////
// Env

std::shared_ptr<mll::Env>
mll::Env::create()
{
    struct Env_ : Env {};
    return std::make_shared<Env_>();
}

std::shared_ptr<mll::Env>
mll::Env::derive_new()
{
    auto derived = create();
    derived->base_ = shared_from_this();
    return derived;
}

void
mll::Env::set(std::string const& name, Node const& value)
{
    vars_[name] = value;
}

bool
mll::Env::update(std::string const& name, Node const& value)
{
    for (auto env = this; env; env = env->base_.get()) {
        auto it = env->vars_.find(name);
        if (it != env->vars_.end()) {
            it->second = value;
            return true;
        }
    }
    return false;
}

std::optional<mll::Node>
mll::Env::lookup(std::string const& name) const
{
    for (auto env = this; env; env = env->base_.get()) {
        auto it = env->vars_.find(name);
        if (it != env->vars_.end()) {
            return it->second;
        }
    }
    return {};
}

std::optional<mll::Node>
mll::Env::shallow_lookup(std::string const& name) const
{
    auto it = vars_.find(name);
    if (it != vars_.end()) {
        return it->second;
    }
    return {};
}

////////////////////////////////////////////////////////////////////////////////
// eval

namespace mll {

    class Evaluator: NodeVisitor {
    public:
        explicit Evaluator(std::shared_ptr<Env> env) : env_(env) { }

        Node evaluate(Node expr)
        {
            expr.accept(*this);
            return result_;
        }

    private:
        void visit(List const& list) override
        {
            if (list.empty()) {
                result_ = nil;
                return;
            }

            auto node = eval(car(list), env_);
            auto proc = to_proc(node);
            if (!proc) {
                std::ostringstream oss;
                BasicPrinter{oss}.print(node);
                throw EvalError(oss.str() + " is not a proc.");
            }

            result_ = proc->call(cdr(list), env_);
        }

        void visit(Number const& num) override
        {
            result_ = num;
        }

        void visit(String const& str) override
        {
            result_ = str;
        }

        void visit(Symbol const& sym) override
        {
            if (sym.name() == MLL_QUOTE) {
                static auto quote_proc = make_proc([](List args, std::shared_ptr<Env>) {
                    return car(args);
                });
                result_ = quote_proc;
            }
            else {
                auto value = env_->lookup(sym.name());
                if (!value) {
                    throw EvalError("Unknown symbol: " + sym.name());
                }
                result_ = *value;
            }
        }

        void visit(Proc const& proc) override
        {
            assert(false);
        }

    private:
        std::shared_ptr<Env> env_;
        Node result_;
    };
}

mll::Node
mll::eval(Node expr, std::shared_ptr<Env> env)
{
    return Evaluator(env).evaluate(expr);
}

////////////////////////////////////////////////////////////////////////////////
// Printer

mll::BasicPrinter::BasicPrinter(std::ostream& ostream)
    : ostream_(ostream)
{
}

void
mll::BasicPrinter::print(Node node)
{
    print(node, true);
}

void
mll::BasicPrinter::print(Node node, bool is_head)
{
    is_head_stack_.push(is_head);
    node.accept(*this);
    is_head_stack_.pop();
}

void
mll::BasicPrinter::visit(List const& list)
{
    if (list.empty()) {
        ostream_ << "()";
        return;
    }

    auto quoted = false;
    auto symbol = to_symbol(car(list));

    if (symbol && symbol->name() == MLL_QUOTE) {
        ostream_ << "'";
        quoted = true;
    }

    if (!quoted && is_head()) {
        ostream_ << '(';
    }
    if (!quoted) {
        auto head = car(list);
        print(head, /* is_head */ true);
    }
    auto tail = cdr(list);
    if (!tail.empty()) {
        if (!quoted) {
            ostream_ << ' ';
        }
        print(tail, /* is_head */ false);
    }
    if (!quoted && is_head()) {
        ostream_ << ')';
    }
}

void
mll::BasicPrinter::visit(Number const& num)
{
    ostream_ << num.value();
}

void
mll::BasicPrinter::visit(String const& str)
{
    ostream_ << quote_text(str.text());
}

void
mll::BasicPrinter::visit(Symbol const& sym)
{
    ostream_ << sym.name();
}

void
mll::BasicPrinter::visit(Proc const& proc)
{
    ostream_ << "<#proc>";
}

bool
mll::BasicPrinter::is_head() const
{
    return is_head_stack_.top();
}
