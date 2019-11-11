#include <mll/eval.hpp>

#include <mll/env.hpp>
#include <mll/node.hpp>
#include <mll/print.hpp>

#include <cassert>

namespace mll {

namespace {
class Evaluator : NodeVisitor {
public:
    explicit Evaluator(Env& env) : env_{env}
    {}

    Node evaluate(Node const& expr)
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
        if (auto proc = dynamic_node_cast<Proc>(node)) {
            result_ = proc->call(cdr(list), env_);
        }
        else {
            throw EvalError(std::to_string(node) + " is not a proc.");
        }
    }

    void visit(Proc const& proc) override
    {
        assert(false);
    }

    void visit(Custom const& custom) override
    {
        result_ = custom;
    }

    void visit(Symbol const& sym) override
    {
        auto value = env_.deep_lookup(sym.name());
        if (!value.has_value()) {
            throw EvalError("Unknown symbol: " + sym.name());
        }
        result_ = *value;
    }

private:
    Env& env_;
    Node result_;
};
} // namespace

Node eval(Node const& expr, Env& env)
{
    return Evaluator{env}.evaluate(expr);
}
} // namespace mll