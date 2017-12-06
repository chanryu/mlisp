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

        void accept(NodeVisitor&, bool) const;

    protected:
        std::shared_ptr<Data const> data_;
    };

    class List: public Node {
    public:
        struct Data;

        List();
        List(List const&);
        List& operator = (List const&);

        friend Node car(List list) noexcept;
        friend List cdr(List list) noexcept;
        friend List cons(Node head, List tail) noexcept;

    private:
        explicit List(std::shared_ptr<Data const>);
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
        virtual void visit(List, bool is_head) = 0;
        virtual void visit(Symbol, bool is_head) = 0;
    };

    class ParseError: public std::runtime_error {
    public:
        explicit ParseError(char const* what);
        explicit ParseError(std::string const& what);
    };
    
    class Parser {
    public:
        Node parse(std::istream& istream);
        bool clean() const noexcept;

    private:
        Symbol intern(std::string text) noexcept;

    private:
        struct Context {
            bool paren;
            Node head;
        };
        std::stack<Context> stack_;
        std::map<std::string, Symbol> symbols_;
    };

    class EvalError: public std::runtime_error {
    public:
        explicit EvalError(char const* what);
        explicit EvalError(std::string const& what);
    };

    Node eval(Node expr, List env);

    Node car(List list) noexcept;
    List cdr(List list) noexcept;
    List cons(Node head, List tail) noexcept;
}
