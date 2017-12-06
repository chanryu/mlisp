#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

namespace mlisp {

    class NodeVisitor;

    class Node {
    public:
        struct Data;

        Node();
        Node(Node const&);
        explicit Node(std::shared_ptr<Data const>);
        Node& operator = (Node const&);

        operator bool() const;

        void accept(NodeVisitor&) const;

    protected:
        std::shared_ptr<Data const> data_;
    };

    class List: public Node {
    public:
        struct Data;

        List();
        List(List const&);
        explicit List(std::shared_ptr<Data const>);
        List& operator = (List const&);

        Node head() const;
        List tail() const;
    };

    class Symbol: public Node {
    public:
        struct Data;

        Symbol();
        Symbol(Symbol const&);
        explicit Symbol(std::shared_ptr<Data const>);
        Symbol& operator = (Symbol const&);

        std::string const& text() const;
    };

    class NodeVisitor {
    public:
        virtual void visit(List) = 0;
        virtual void visit(Symbol) = 0;
    };

    using ParseError = std::runtime_error;
    
    class Parser {
    public:
        Node parse(std::istream& istream);
        bool clean() const noexcept;

    private:
        Symbol intern(std::string text) noexcept;

    private:
        struct Context {
            bool paran;
            Node head;
        };
        std::stack<Context> stack_;
        std::map<std::string, Symbol> symbols_;
    };

    Node car(List list) noexcept;
    List cdr(List list) noexcept;
    List cons(Node head, List tail) noexcept;

    using EvalError = std::runtime_error;
    Node eval(Node expr, List env);
}
