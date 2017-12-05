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
        virtual ~Node();
        virtual void accept(NodeVisitor&) const = 0;
    };

    class Atom: public Node {
    };

    class Symbol: public Atom {
    public:
        explicit Symbol(std::string text) noexcept;
        void accept(NodeVisitor& visitor) const override;
        
        std::string const text;
    };

    class List: public Node {
    public:
        List(std::shared_ptr<Node> head, std::shared_ptr<List> tail) noexcept;
        void accept(NodeVisitor& visitor) const override;
        
        std::shared_ptr<Node> const head;
        std::shared_ptr<List> const tail;
    };

    class NodeVisitor {
    public:
        virtual void visit(Symbol const&) = 0;
        virtual void visit(List const&) = 0;
    };

    std::shared_ptr<Node> eval(std::shared_ptr<Node> expr, std::shared_ptr<List> env);

    class Parser {
    public:
        std::shared_ptr<Node> parse(std::istream& istream);
        bool clean() const noexcept;

    private:
        std::shared_ptr<Symbol> intern(std::string text) noexcept;

    private:
        struct Context {
            bool paran;
            std::shared_ptr<Node> node;
        };
        std::stack<Context> stack_;
        std::map<std::string, std::shared_ptr<Symbol>> symbols_;
    };
}
