#include <cassert>

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

    void skip_spaces(std::istream& istream) noexcept
    {
        while (is_space(istream.peek())) {
            istream.get();
        }
    }

    bool parse_token(std::istream& istream, std::string& token) noexcept
    {
        skip_spaces(istream);
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

    char const* const MLISP_QUOTE = "mlisp:quote";
}

////////////////////////////////////////////////////////////////////////////////
// Node::Data

struct mlisp::Node::Data: public std::enable_shared_from_this<Data> {
    virtual ~Data() {}
    virtual void accept(NodeVisitor&) const = 0;
};


////////////////////////////////////////////////////////////////////////////////
// List::Data

struct mlisp::List::Data: public mlisp::Node::Data {

    Data(Node h, List t) noexcept : head{h}, tail{t} { }

    void accept(NodeVisitor& visitor) const override
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

    void accept(NodeVisitor& visitor) const override
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

    void accept(NodeVisitor& visitor) const override
    {
        visitor.visit(Number{
            std::static_pointer_cast<Data const>(shared_from_this())
        });
    }
    
    double const value;
};

////////////////////////////////////////////////////////////////////////////////
// Symbol::Data

struct mlisp::Symbol::Data: public mlisp::Node::Data {
    
    explicit Data(std::string n) noexcept : name{std::move(n)} { }

    void accept(NodeVisitor& visitor) const override
    {
        visitor.visit(Symbol{
            std::static_pointer_cast<Data const>(shared_from_this())
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
        throw TypeError("XXX is not a List");
    }
    return { list_data };
}

mlisp::Proc
mlisp::Node::to_proc() const
{
    auto proc_data = std::dynamic_pointer_cast<Proc::Data const>(data_);
    if (!proc_data) {
        throw TypeError("XXX is not a Proc");
    }
    return { proc_data };
}

mlisp::Number
mlisp::Node::to_number() const
{
    auto number_data = std::dynamic_pointer_cast<Number::Data const>(data_);
    if (!number_data) {
        throw TypeError("XXX is not a Number");
    }
    return { number_data };
}

mlisp::Symbol
mlisp::Node::to_symbol() const
{
    auto symbol_data = std::dynamic_pointer_cast<Symbol::Data const>(data_);
    if (!symbol_data) {
        throw TypeError("XXX is not a Symbol");
    }
    return { symbol_data };
}

////////////////////////////////////////////////////////////////////////////////
// List

mlisp::List::List() noexcept
{
}

mlisp::List::List(List const& other) noexcept
    : Node{other.data_}
{
}

mlisp::List::List(std::shared_ptr<Data const> data) noexcept
    : Node{data}
{
}

mlisp::List&
mlisp::List::operator = (List const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

////////////////////////////////////////////////////////////////////////////////
// Proc

mlisp::Proc::Proc(Func func) noexcept
    : Node{ std::make_shared<Data>(func) }
{
}

mlisp::Proc::Proc(Proc const& other) noexcept
    : Node{other.data_}
{
}

mlisp::Proc::Proc(std::shared_ptr<Data const> data) noexcept
    : Node{data}
{
}

mlisp::Node
mlisp::Proc::operator()(List args, List env) const
{
    auto const& func = static_cast<Data const*>(data_.get())->func;
    if (func) {
        return func(args, env);
    }
    return {};  // nil
}

////////////////////////////////////////////////////////////////////////////////
// Number

mlisp::Number::Number(double value) noexcept
    : Node{ std::make_shared<Data>(value)}
{
}

mlisp::Number::Number(Number const& other) noexcept
    : Node{other.data_}
{
}

mlisp::Number::Number(std::shared_ptr<Data const> data) noexcept
    : Node{data}
{
}

double
mlisp::Number::value() const
{
    return static_cast<Data const*>(data_.get())->value;
}

////////////////////////////////////////////////////////////////////////////////
// Symbol

mlisp::Symbol::Symbol(std::string name) noexcept
    : Node{ std::make_shared<Data>(std::move(name))}
{
}

mlisp::Symbol::Symbol(Symbol const& other) noexcept
    : Node{other.data_}
{
}

mlisp::Symbol::Symbol(std::shared_ptr<Data const> data) noexcept
    : Node{data}
{
}

bool
mlisp::Symbol::operator == (Node const& rhs) noexcept
{
    if (rhs) {
        try {
            auto sym = rhs.to_symbol();
            return name() == sym.name();
        }
        catch (TypeError& e) {
            return false;
        }
    }
    return false;
}

std::string const&
mlisp::Symbol::name() const
{
    return static_cast<Data const*>(data_.get())->name;
}


///////////////////////////////////////////////////////////////////////////////
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
        else if (token[0] == '.' || (token[0] >= '0' && token[0] <= '9')) {
            node = Number{ std::stod(token) };
        }
        else {
            node = Symbol{ std::move(token) };
        }

        while (true) {
            if (stack_.empty()) {
                expr = node;
                return true;
            }

            if (stack_.top().type == Context::Type::quote) {
                stack_.pop();
                node = cons(Symbol(MLISP_QUOTE), cons(node, {}));
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

///////////////////////////////////////////////////////////////////////////////
// eval

mlisp::Node
mlisp::eval(Node expr, List env)
{
    class NodeEvaluator: NodeVisitor {
    public:
        explicit NodeEvaluator(List env)
        {
            auto quote_proc = Proc{[] (List args, List) {
                return car(args);
            }};
            env_ = cons(Symbol{MLISP_QUOTE}, cons(quote_proc, env));
        }

        Node evaluate(Node expr)
        {
            expr.accept(*this);

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
            for (auto env = env_; env; env = cdr(env)) {
                if (symbol == car(env)) {
                    result_ = car(cdr(env));
                    return;
                }
            }

            throw EvalError("Unknown symbol: " + symbol.name());
        }

    private:
        List env_;
        Node result_;
    };

    return NodeEvaluator(env).evaluate(expr);
}

///////////////////////////////////////////////////////////////////////////////
// Builtins

mlisp::Node
mlisp::car(List list) noexcept
{
    if (list.data_) {
        return static_cast<List::Data const*>(list.data_.get())->head;
    }
    return {};
}

mlisp::List
mlisp::cdr(List list) noexcept
{
    if (list.data_) {
        return static_cast<List::Data const*>(list.data_.get())->tail;
    }
    return {};
}

mlisp::List
mlisp::cons(Node head, List tail) noexcept
{
    return List{ std::make_shared<List::Data>(head, tail) };
}

namespace {
    using namespace mlisp;

    void print_node(std::ostream& ostream, Node const& node, bool is_head) {

        class NodePrinter: NodeVisitor {
        public:
            NodePrinter(std::ostream& ostream, bool is_head)
                : ostream_(ostream), is_head_(is_head) {}

            void print(Node const& node)
            {
                node.accept(*this);
            }

        private:
            void visit(List list) override
            {
                auto print_parens = is_head_;
                auto print_head = true;

                try {
                    auto symbol = car(list).to_symbol();
                    if (symbol.name() == MLISP_QUOTE) {
                        ostream_ << "'";
                        print_parens = false;
                        print_head = false;
                    }
                }
                catch (TypeError&) {
                    // ignore and continue
                }

                if (print_parens) {
                    ostream_ << '(';
                }
                if (print_head) {
                    auto head = car(list);
                    print_node(ostream_, head, true);
                }
                auto tail = cdr(list);
                if (tail) {
                    if (print_head) {
                        ostream_ << ' ';
                    }
                    print_node(ostream_, tail, false);
                }                
                if (print_parens) {
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
                ostream_ << symbol.name();
            }

        private:
            std::ostream& ostream_;
            bool is_head_;
        };
        
        if (node) {
            NodePrinter{ostream, is_head}.print(node);
        }
        else {
            ostream << "nil";
        }
    }
}

std::ostream&
std::operator << (std::ostream& ostream, mlisp::Node const& node)
{
    print_node(ostream, node, true);
    return ostream;
}
