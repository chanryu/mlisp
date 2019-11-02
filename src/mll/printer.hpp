#pragma once

#include <mll/node.hpp>

#include <ostream>
#include <stack>
#include <vector>

namespace mll {

class BasicPrinter: NodeVisitor {
public:
    explicit BasicPrinter(std::ostream& ostream);

    void print(Node const&);

    void visit(List const&) override;
    void visit(Proc const&) override;
    void visit(Number const&) override;
    void visit(String const&) override;
    void visit(Symbol const&) override;

private:
    bool is_head() const;
    void print(Node const&, bool is_head);

    std::ostream& ostream_;
    std::stack<bool, std::vector<bool>> is_head_stack_;
};

}