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

    bool parse_token(std::istream& istream, std::string& token) noexcept
    {
        skip_whitespaces_and_comments(istream);
        token.clear();

        char c;
        while (istream.get(c)) {
            if (is_space(c)) {
                assert(!token.empty());
                return true;
            }

            if (is_quote(c)) {
                token.push_back(c);
                return true;
            }

            if (is_paren(c)) {
                if (token.empty()) {
                    token.push_back(c);
                }
                else {
                    istream.unget();
                }
                return true;
            }
            token.push_back(c);
        }

        return !token.empty();
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

    char const* const MLISP_KEYWORD_NIL = "nil";
    char const* const MLISP_BUILTIN_QUOTE = "mlisp-built-in:quote";
}

////////////////////////////////////////////////////////////////////////////////
// NodeData

struct mlisp::detail::NodeData: public std::enable_shared_from_this<NodeData> {
    virtual ~NodeData() {}
    virtual void accept(NodeVisitor&) const = 0;
};


////////////////////////////////////////////////////////////////////////////////
// ListData

struct mlisp::detail::ListData: public mlisp::detail::NodeData {

    ListData(Node h, List t) noexcept : head{h}, tail{t} { }

    void accept(NodeVisitor& visitor) const override
    {
        visitor.visit(List{
            std::static_pointer_cast<ListData const>(shared_from_this())
        });
    }
    
    Node const head;
    List const tail;
};

////////////////////////////////////////////////////////////////////////////////
// ProcData

struct mlisp::detail::ProcData: public mlisp::detail::NodeData {

    explicit ProcData(Func f) noexcept : func{f} { }

    void accept(NodeVisitor& visitor) const override
    {
        visitor.visit(Proc{
            std::static_pointer_cast<ProcData const>(shared_from_this())
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

mlisp::Node::Node(Symbol const& symbol) noexcept
    : data_{symbol.data_}
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
mlisp::Node::accept(NodeVisitor& visitor) const
{
    if (data_) {
        data_->accept(visitor);
    }
}

mlisp::List
mlisp::Node::to_list() const
{
    if (!data_) {
        return {}; // nil
    }

    auto list_data = std::dynamic_pointer_cast<List::Data const>(data_);
    if (!list_data) {
        throw EvalError(std::to_string(*this) + " is not a list");
    }
    return { list_data };
}

mlisp::Proc
mlisp::Node::to_proc() const
{
    auto proc_data = std::dynamic_pointer_cast<Proc::Data const>(data_);
    if (!proc_data) {
        throw EvalError(std::to_string(*this) + " is not a procedure");
    }
    return { proc_data };
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

mlisp::Symbol
mlisp::Node::to_symbol() const
{
    auto symbol_data = std::dynamic_pointer_cast<Symbol::Data const>(data_);
    if (!symbol_data) {
        throw EvalError(std::to_string(*this) + " is not a symbol");
    }
    return { symbol_data };
}

bool
mlisp::Node::is_list() const
{
    if (!data_) {
        return true;
    }
    return !!dynamic_cast<List::Data const *>(data_.get());
}

bool
mlisp::Node::is_symbol() const
{
    return !!dynamic_cast<Symbol::Data const *>(data_.get());
}

////////////////////////////////////////////////////////////////////////////////
// List

mlisp::List::List() noexcept
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

mlisp::List&
mlisp::List::operator = (List const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

mlisp::List::operator bool() const noexcept
{
    return !!data_;
}

////////////////////////////////////////////////////////////////////////////////
// Proc

mlisp::Proc::Proc(Proc const& other) noexcept
    : data_{other.data_}
{
}

mlisp::Proc::Proc(std::shared_ptr<Data const> data) noexcept
    : data_{data}
{
}

mlisp::Node
mlisp::Proc::operator()(List args, std::shared_ptr<Env> env) const
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
    std::string token;

    while (parse_token(istream, token)) {

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

bool
mlisp::read(std::istream& istream, Node& expr)
{
    if (istream.good()) {
        auto pos = istream.tellg();
        if (Parser{}.parse(istream, expr)) {
            return true;
        }
        istream.clear();
        istream.seekg(pos);
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
// Env

struct mlisp::Env {
    std::shared_ptr<Env> base;
    std::map<std::string, Node> vars;
};

std::shared_ptr<Env>
mlisp::make_env(std::shared_ptr<Env> base_env, std::map<std::string, Node> vars)
{
    auto env = std::make_shared<Env>();
    env->base = base_env;
    return env;
}

void
mlisp::set(std::shared_ptr<Env> env, std::string name, Node value)
{
    assert(env);

    if (name == MLISP_KEYWORD_NIL) {
        throw EvalError("`nil' cannot be redefined.");
    }

    env->vars[name] = value;
}

bool
mlisp::lookup(std::shared_ptr<Env> env, std::string const& name, Node& node)
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
        explicit NodeEvaluator(std::shared_ptr<Env> env) : env_(env)
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
            auto proc = cmd.to_proc();

            result_ = proc(cdr(list), env_);
        }

        void visit(Proc proc) override
        {
            assert(false);
        }

        void visit(Number number) override
        {
            result_ = number;
        }

        void visit(Symbol symbol) override
        {
            if (symbol.name() == MLISP_KEYWORD_NIL) {
                result_ = Node{};
            }
            else if (symbol.name() == MLISP_BUILTIN_QUOTE) {
                static auto quote_proc = proc([] (List args, std::shared_ptr<Env>) {
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

    private:
        std::shared_ptr<Env> env_;
        Node result_;
    };
}

mlisp::Node
mlisp::eval(Node expr, std::shared_ptr<Env> env)
{
    return NodeEvaluator(env).evaluate(expr);
}

////////////////////////////////////////////////////////////////////////////////
// Built-ins

mlisp::Node
mlisp::car(List list) noexcept
{
    return list.data_ ? list.data_->head : Node{};
}

mlisp::List
mlisp::cdr(List list) noexcept
{
    return list.data_ ? list.data_->tail : List{};
}

mlisp::List
mlisp::cons(Node head, List tail) noexcept
{
    return List{ std::make_shared<List::Data>(head, tail) };
}

mlisp::Proc
mlisp::proc(Func func) noexcept
{
    return Proc{ std::make_shared<Proc::Data>(func) };
}

mlisp::Number
mlisp::number(double value) noexcept
{
    return Number{ std::make_shared<Number::Data>(value) };
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
        void visit(List list) override
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

        void visit(Proc proc) override
        {
            ostream_ << "<#proc>";
        }

        void visit(Number number) override
        {
            ostream_ << number.value();
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

    private:
        std::ostream& ostream_;
        bool is_head_;
    };

    void print_node(std::ostream& ostream, Node const& node, bool is_head)
    {
        NodePrinter{ostream, is_head}.print(node);
    }
}

std::ostream&
mlisp::operator << (std::ostream& ostream, mlisp::Node const& node)
{
    print_node(ostream, node, true);
    return ostream;
}

std::ostream&
mlisp::operator << (std::ostream& ostream, mlisp::List const& list)
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
