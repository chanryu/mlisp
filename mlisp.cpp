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
// NodeData

struct mlisp::detail::NodeData: public std::enable_shared_from_this<NodeData> {
    virtual ~NodeData() {}
    virtual void accept(NodeVisitor&) const = 0;
};


////////////////////////////////////////////////////////////////////////////////
// ConsData

struct mlisp::detail::ConsData: public mlisp::detail::NodeData {

    ConsData(Node h, Cons t) noexcept : head{h}, tail{t} { }

    void accept(NodeVisitor& visitor) const override
    {
        visitor.visit(Cons{
            std::static_pointer_cast<ConsData const>(shared_from_this())
        });
    }

    Node const head;
    Cons const tail;
};

////////////////////////////////////////////////////////////////////////////////
// ProcedureData

struct mlisp::detail::ProcedureData: public mlisp::detail::NodeData {

    explicit ProcedureData(Func f) noexcept : func{f} { }

    void accept(NodeVisitor& visitor) const override
    {
        visitor.visit(Procedure{
            std::static_pointer_cast<ProcedureData const>(shared_from_this())
        });
    }

    Func const func;
};

////////////////////////////////////////////////////////////////////////////////
// NumberData

struct mlisp::detail::NumberData: public mlisp::detail::NodeData {

    explicit NumberData(double v) noexcept : value{v} { }

    void accept(NodeVisitor& visitor) const override
    {
        visitor.visit(Number{
            std::static_pointer_cast<NumberData const>(shared_from_this())
        });
    }

    double const value;
};

////////////////////////////////////////////////////////////////////////////////
// StringData

struct mlisp::detail::StringData: public mlisp::detail::NodeData {

    explicit StringData(std::string t) noexcept : text{std::move(t)} { }

    void accept(NodeVisitor& visitor) const override
    {
        visitor.visit(String{
            std::static_pointer_cast<StringData const>(shared_from_this())
        });
    }

    std::string const text;
};

////////////////////////////////////////////////////////////////////////////////
// SymbolData

struct mlisp::detail::SymbolData: public mlisp::detail::NodeData {

    explicit SymbolData(std::string n) noexcept : name{std::move(n)} { }

    void accept(NodeVisitor& visitor) const override
    {
        visitor.visit(Symbol{
            std::static_pointer_cast<SymbolData const>(shared_from_this())
        });
    }

    std::string const name;
};

////////////////////////////////////////////////////////////////////////////////
// Node

mlisp::Node::Node() noexcept
{
}

mlisp::Node::Node(Node const& other) noexcept
    : data_{other.data_}
{
}

mlisp::Node::Node(Cons const& list) noexcept
    : data_{list.data_}
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

mlisp::Node::Node(Procedure const& proc) noexcept
: data_{proc.data_}
{
}

mlisp::Node::Node(std::shared_ptr<Data const> data) noexcept
    : data_{data}
{
}

mlisp::Node&
mlisp::Node::operator = (Node const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mlisp::Node&
mlisp::Node::operator = (Cons const& rhs) noexcept
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

mlisp::Node&
mlisp::Node::operator = (Procedure const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mlisp::Node::operator bool() const noexcept
{
    return !!data_;
}

void
mlisp::Node::accept(NodeVisitor& visitor) const
{
    if (data_) {
        data_->accept(visitor);
    }
}

mlisp::Cons
mlisp::Node::to_cons() const
{
    if (!data_) {
        return {}; // nil
    }

    auto list_data = std::dynamic_pointer_cast<Cons::Data const>(data_);
    if (!list_data) {
        throw EvalError(std::to_string(*this) + " is not a list");
    }
    return { list_data };
}

mlisp::Number
mlisp::Node::to_number() const
{
    auto number_data = std::dynamic_pointer_cast<Number::Data const>(data_);
    if (!number_data) {
        throw EvalError(std::to_string(*this) + " is not a number");
    }
    return { number_data };
}

mlisp::String
mlisp::Node::to_string() const
{
    auto string_data = std::dynamic_pointer_cast<String::Data const>(data_);
    if (!string_data) {
        throw EvalError(std::to_string(*this) + " is not a string");
    }
    return { string_data };
}

mlisp::Symbol
mlisp::Node::to_symbol() const
{
    auto symbol_data = std::dynamic_pointer_cast<Symbol::Data const>(data_);
    if (!symbol_data) {
        throw EvalError(std::to_string(*this) + " is not a symbol");
    }
    return { symbol_data };
}

mlisp::Procedure
mlisp::Node::to_procedure() const
{
    auto proc_data = std::dynamic_pointer_cast<Procedure::Data const>(data_);
    if (!proc_data) {
        throw EvalError(std::to_string(*this) + " is not a procedure");
    }
    return { proc_data };
}

bool
mlisp::Node::is_cons() const
{
    if (!data_) {
        return true;
    }
    return !!dynamic_cast<Cons::Data const *>(data_.get());
}

bool
mlisp::Node::is_number() const
{
    return !!dynamic_cast<Number::Data const *>(data_.get());
}

bool
mlisp::Node::is_string() const
{
    return !!dynamic_cast<String::Data const *>(data_.get());
}

bool
mlisp::Node::is_symbol() const
{
    return !!dynamic_cast<Symbol::Data const *>(data_.get());
}

bool
mlisp::Node::is_procedure() const
{
    return !!dynamic_cast<Procedure::Data const *>(data_.get());
}

////////////////////////////////////////////////////////////////////////////////
// Cons

mlisp::Cons::Cons() noexcept
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

mlisp::Procedure::Procedure(Procedure const& other) noexcept
    : data_{other.data_}
{
}

mlisp::Procedure::Procedure(std::shared_ptr<Data const> data) noexcept
    : data_{data}
{
}

mlisp::Node
mlisp::Procedure::operator()(Cons args, EnvPtr env) const
{
    if (data_->func) {
        return data_->func(args, env);
    }
    return {};  // nil
}

////////////////////////////////////////////////////////////////////////////////
// Number

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
mlisp::Parser::parse(std::istream& istream, Node& expr)
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
                    list = cons(c.head, list);
                }
                if (c.type == Context::Type::paren) {
                    break;
                }
            }
            node = list;
        }
        else if (is_number(token)) {
            node = number(std::stod(token));
        }
        else {
            node = symbol(std::move(token));
        }

        while (true) {
            if (stack_.empty()) {
                expr = node;
                return true;
            }

            if (stack_.top().type == Context::Type::quote) {
                stack_.pop();
                node = cons(symbol(MLISP_BUILTIN_QUOTE), cons(node, {}));
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
    std::map<std::string, Node> vars;
};

EnvPtr
mlisp::make_env(EnvPtr base_env)
{
    auto env = std::make_shared<Env>();
    env->base = base_env;
    return env;
}

void
mlisp::set(EnvPtr env, std::string name, Node value)
{
    assert(env);
    env->vars[name] = value;
}

bool
mlisp::update(EnvPtr env, std::string const& name, Node node)
{
    assert(env);

    auto i = env->vars.find(name);
    if (i != env->vars.end()) {
        i->second = node;
        return true;
    }
    return false;
}

bool
mlisp::lookup(EnvPtr env, std::string const& name, Node& node)
{
    while (env) {
        auto i = env->vars.find(name);
        if (i != env->vars.end()) {
            node = i->second;
            return true;
        }
        env = env->base;
    }

    return false;
}


////////////////////////////////////////////////////////////////////////////////
// eval

namespace {
    class NodeEvaluator: NodeVisitor {
    public:
        explicit NodeEvaluator(EnvPtr env) : env_(env)
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
        void visit(Cons list) override
        {
            auto cmd = eval(car(list), env_);
            auto proc = cmd.to_procedure();

            result_ = proc(cdr(list), env_);
        }

        void visit(Number number) override
        {
            result_ = number;
        }

        void visit(String string) override
        {
            result_ = string;
        }

        void visit(Symbol symbol) override
        {
            if (symbol.name() == MLISP_BUILTIN_QUOTE) {
                static auto quote_proc = procedure([] (Cons args, EnvPtr) {
                    return car(args);
                });
                result_ = quote_proc;
            }
            else {
                Node value;
                if (lookup(env_, symbol.name(), value)) {
                    result_ = value;
                } else {
                    throw EvalError("Unknown symbol: " + symbol.name());
                }
            }
        }

        void visit(Procedure proc) override
        {
            assert(false);
        }

    private:
        EnvPtr env_;
        Node result_;
    };
}

mlisp::Node
mlisp::eval(Node expr, EnvPtr env)
{
    return NodeEvaluator(env).evaluate(expr);
}

////////////////////////////////////////////////////////////////////////////////
// Built-ins

mlisp::Node
mlisp::car(Cons list) noexcept
{
    return list.data_ ? list.data_->head : Node{};
}

mlisp::Cons
mlisp::cdr(Cons list) noexcept
{
    return list.data_ ? list.data_->tail : Cons{};
}

mlisp::Cons
mlisp::cons(Node head, Cons tail) noexcept
{
    return Cons{ std::make_shared<Cons::Data>(head, tail) };
}

mlisp::Procedure
mlisp::procedure(Func func) noexcept
{
    return Procedure{ std::make_shared<Procedure::Data>(func) };
}

mlisp::Number
mlisp::number(double value) noexcept
{
    return Number{ std::make_shared<Number::Data>(value) };
}

mlisp::String
mlisp::string(std::string text) noexcept
{
    return String{ std::make_shared<String::Data>(std::move(text)) };
}

mlisp::Symbol
mlisp::symbol(std::string name) noexcept
{
    return Symbol{ std::make_shared<Symbol::Data>(std::move(name)) };
}

////////////////////////////////////////////////////////////////////////////////
// Printer

namespace {
    using namespace mlisp;

    class NodePrinter: NodeVisitor {
    public:
        NodePrinter(std::ostream& ostream, bool is_head)
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
        void visit(Cons list) override
        {
            auto quoted = false;

            try {
                auto symbol = car(list).to_symbol();
                if (symbol.name() == MLISP_BUILTIN_QUOTE) {
                    ostream_ << "'";
                    quoted = true;
                }
            }
            catch (EvalError&) {
                // ignore and continue
            }

            if (!quoted && is_head_) {
                ostream_ << '(';
            }
            if (!quoted) {
                auto head = car(list);
                NodePrinter{ostream_, true}.print(head);
            }
            auto tail = cdr(list);
            if (tail) {
                if (!quoted) {
                    ostream_ << ' ';
                }
                NodePrinter{ostream_, false}.print(tail);
            }
            if (!quoted && is_head_) {
                ostream_ << ')';
            }
        }

        void visit(Number number) override
        {
            ostream_ << number.value();
        }

        void visit(String string) override
        {
            ostream_ << string.text();
        }

        void visit(Symbol symbol) override
        {
            if (symbol.name() == MLISP_BUILTIN_QUOTE) {
                ostream_ << "quote";
            }
            else {
                ostream_ << symbol.name();
            }
        }

        void visit(Procedure proc) override
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
    NodePrinter{ostream, true}.print(node);
    return ostream;
}

std::ostream&
mlisp::operator << (std::ostream& ostream, mlisp::Cons const& list)
{
    return ostream << Node{ list };
}

std::string
std::to_string(mlisp::Node const& node)
{
    ostringstream ss;
    ss << node;
    return ss.str();
}
