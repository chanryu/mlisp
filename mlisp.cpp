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

    void skip_spaces(std::istream& istream) noexcept
    {
        while (is_space(istream.peek())) {
            istream.get();
        }
    }

    std::string get_token(std::istream& istream) noexcept
    {
        skip_spaces(istream);

        std::string token;

        char c;
        while (istream.get(c)) {
            if (is_space(c)) {
                assert(!token.empty());
                break;
            }
            if (is_paren(c)) {
                if (token.empty()) {
                    token.push_back(c);
                }
                else {
                    istream.unget();
                }
                break;
            }
            token.push_back(c);
        }

        return token;
    }

    void debug_print(mlisp::Node node)
    {
        std::cout << node << std::endl << std::flush;
    }

    void debug_print(std::string text)
    {
        std::cout << text << std::endl << std::flush;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Node::Data

struct mlisp::Node::Data: public std::enable_shared_from_this<Data> {
    virtual ~Data() {}
    virtual void accept(NodeVisitor&) const = 0;
};


///////////////////////////////////////////////////////////////////////////////
// List::Data

struct mlisp::List::Data: public mlisp::Node::Data {

    Data(Node h, List t) noexcept : head{h}, tail{t}
    {
        assert(head || (!head && !tail));
    }

    void accept(NodeVisitor& visitor) const override
    {
        visitor.visit(List{ std::static_pointer_cast<Data const>(shared_from_this()) });
    }
    
    Node const head;
    List const tail;
};

///////////////////////////////////////////////////////////////////////////////
// Proc::Data

struct mlisp::Proc::Data: public mlisp::Node::Data {

    explicit Data(Func f) noexcept : func{f}
    {
    }

    void accept(NodeVisitor& visitor) const override
    {
        visitor.visit(Proc{ std::static_pointer_cast<Data const>(shared_from_this()) });
    }

    Func const func;
};

///////////////////////////////////////////////////////////////////////////////
// Number::Data

struct mlisp::Number::Data: public mlisp::Node::Data {
    
    explicit Data(double v) noexcept : value{v}
    {
    }

    void accept(NodeVisitor& visitor) const override
    {
        visitor.visit(Number{ std::static_pointer_cast<Data const>(shared_from_this()) });
    }
    
    double const value;
};

///////////////////////////////////////////////////////////////////////////////
// Symbol::Data

struct mlisp::Symbol::Data: public mlisp::Node::Data {
    
    explicit Data(std::string n) noexcept : name{std::move(n)}
    {
        assert(!name.empty());
    }

    void accept(NodeVisitor& visitor) const override
    {
        visitor.visit(Symbol{ std::static_pointer_cast<Data const>(shared_from_this()) });
    }
    
    std::string const name;
};

///////////////////////////////////////////////////////////////////////////////
// Node

mlisp::Node::Node() noexcept {}
mlisp::Node::Node(Node const& other) noexcept : data_{other.data_} {}
mlisp::Node::Node(std::shared_ptr<Data const> data) noexcept : data_{data} {}

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
mlisp::Node::to_list() const noexcept
{
    auto list_data = std::dynamic_pointer_cast<List::Data const>(data_);
    if (list_data) {
        return cons(list_data->head, list_data->tail);
    }

    return {};
}

mlisp::Proc
mlisp::Node::to_proc() const noexcept
{
    return { std::dynamic_pointer_cast<Proc::Data const>(data_) };
}

mlisp::Number
mlisp::Node::to_number() const noexcept
{
    return { std::dynamic_pointer_cast<Number::Data const>(data_) };
}

mlisp::Symbol
mlisp::Node::to_symbol() const noexcept
{
    return { std::dynamic_pointer_cast<Symbol::Data const>(data_) };
}

///////////////////////////////////////////////////////////////////////////////
// List

mlisp::List::List() noexcept {}
mlisp::List::List(List const& other) noexcept : Node{other.data_} {}
mlisp::List::List(std::shared_ptr<Data const> data) noexcept : Node{data} {}

mlisp::List&
mlisp::List::operator = (List const& rhs) noexcept
{
    data_ = rhs.data_;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////
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
    assert(data_);
    auto const& func = static_cast<Data const*>(data_.get())->func;
    return func(args, env);
}

///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
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

std::string const&
mlisp::Symbol::name() const
{
    return static_cast<Data const*>(data_.get())->name;
}

///////////////////////////////////////////////////////////////////////////////
// Parser

Node
mlisp::Parser::parse(std::istream& istream)
{
    std::string token;

    while (true) {
        token = get_token(istream);

        if (token.empty()) {
            break;
        }

        if (token == "(") {
            stack_.push({ true, {} });
            continue;
        }

        Node expr;

        if (token == ")") {

            List list;
            while (true) {
                if (stack_.empty()) {
                    throw ParseError{"Unexpected ')'"};
                }

                auto c = stack_.top();
                stack_.pop();
                list = cons(c.head, list);
                if (c.paren) {
                    break;
                }
            }
            expr = list;
        }
        else if (token[0] == '.' || (token[0] >= '0' && token[0] <= '9')) {
            expr = Number{ std::stod(token) };
        }
        else {
            expr = Symbol{ std::move(token) };
        }
            
        if (stack_.empty()) {
            return expr;
        }
        else {
            if (stack_.top().head) {
                stack_.push({ false, expr });
            } else {
                stack_.top().head = expr;
            }
        }
    }

    return {};
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
        explicit NodeEvaluator(List env) : env_(env)
        {
        }

        Node evaluate(Node expr)
        {
            expr.accept(*this);

            return result_;
        }

    private:
        void visit(List list) override
        {
            static auto const nil = cons({}, {});

            auto head = car(list);
            auto tail = cdr(list);

            auto e_head = eval(head, env_);

            auto proc = e_head.to_proc();

            if (!proc) {
                throw EvalError("xxx not a proc");
            }

            result_ = proc(tail, env_);

            /*
            auto symbol = head.to_symbol();

            if (symbol) {
                auto name = symbol.name();
                if (name == "car" || name == "cdr") {

                    if (cdr(tail)) {
                        throw EvalError(name + ": too many args given");
                    }

                    List arg = eval(car(tail), env_).to_list();
                    if (!arg) {
                        throw EvalError(name + ": must be given a list");
                    }

                    if (name == "car") {
                        result_ = car(arg);
                    }
                    else {
                        result_ = cdr(arg);
                    }
                }
                else {
                    result_ = nil;
                }
            }
            else {
                result_ = nil;
            }
            */
        }

        void visit(Proc proc) override
        {
        }

        void visit(Number number) override
        {
            result_ = number;
        }

        void visit(Symbol symbol) override
        {
            for (auto env = env_; env; env = cdr(env)) {
    
            }
            result_ = symbol;
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
    assert(list.data_);
    return static_cast<List::Data const*>(list.data_.get())->head;
}

mlisp::List
mlisp::cdr(List list) noexcept
{
    assert(list.data_);
    return static_cast<List::Data const*>(list.data_.get())->tail;
}

mlisp::List
mlisp::cons(Node head, List tail) noexcept
{
    if (!head) {
        static List nil{ std::make_shared<List::Data>(Node{}, List{}) };
        return nil;
    }

    return List{ std::make_shared<List::Data>(head, tail) };
}

std::ostream&
std::operator << (std::ostream& os, mlisp::Node const& node)
{
    using namespace mlisp;

    class NodePrinter: NodeVisitor {
    public:
        explicit NodePrinter(std::ostream& ostream) : ostream_(ostream)
        {
        }

        void print(Node const& node)
        {
            is_head_.push(true);
            node.accept(*this);
            is_head_.pop();
        }

    private:
        void visit(List list) override
        {
            auto head = car(list);
            if (!head) {
                ostream_ << "nil";
                return;
            }

            if (is_head_.top()) {
                ostream_ << '(';
            }

            is_head_.push(true);
            head.accept(*this);
            is_head_.pop();

            auto tail = cdr(list);
            if (tail) {
                ostream_ << ' ';

                is_head_.push(false);
                tail.accept(*this);
                is_head_.pop();
            }
            else {
                ostream_ << ')';
            }
        }

        void visit(Proc proc) override
        {
            //ostream_ << proc;
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
        detail::Stack<bool> is_head_;
        std::ostream& ostream_;
    };

    NodePrinter{os}.print(node);
    return os;
}
