#include <mll/eval.hpp>

#include <mll/env.hpp>
#include <mll/list.hpp>
#include <mll/print.hpp>
#include <mll/proc.hpp>
#include <mll/symbol.hpp>

#include <cassert>

namespace mll {

namespace {
class Evaluator : NodeVisitor {
public:
    explicit Evaluator(Env& env) : _env{env}
    {}

    Node evaluate(Node const& expr)
    {
        expr.accept(*this);
        return _result;
    }

private:
    void visit(List const& list) override
    {
        if (list.empty()) {
            _result = nil;
            return;
        }

        auto node = eval(car(list), _env);
        if (auto proc = dynamic_node_cast<Proc>(node)) {
            _result = proc->call(cdr(list), _env);
        }
        else {
            throw EvalError(std::to_string(node) + " is not a proc.");
        }
    }

    void visit(Proc const& /*proc*/) override
    {
        assert(false);
    }

    void visit(Custom const& custom) override
    {
        _result = custom;
    }

    void visit(Symbol const& sym) override
    {
        auto value = _env.deep_lookup(sym.name());
        if (!value.has_value()) {
            throw EvalError("Unknown symbol: " + sym.name());
        }
        _result = *value;
    }

private:
    Env& _env;
    Node _result;
};
} // namespace

Node eval(Node const& expr, Env& env)
{
    return Evaluator{env}.evaluate(expr);
}
} // namespace mll