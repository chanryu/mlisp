#include <mll/eval.hpp>

#include <mll/env.hpp>
#include <mll/node.hpp>
#include <mll/printer.hpp>
#include <mll/symdef.hpp>

#include <cassert>
#include <sstream>

namespace mll {

namespace {
class Evaluator: NodeVisitor {
public:
    explicit Evaluator(Env& env) : env_(env) { }

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
        auto proc = to_proc(node);
        if (!proc) {
            std::ostringstream oss;
            BasicPrinter{oss}.print(node);
            throw EvalError(oss.str() + " is not a proc.");
        }

        result_ = proc->call(cdr(list), env_);
    }

    void visit(Number const& num) override
    {
        result_ = num;
    }

    void visit(String const& str) override
    {
        result_ = str;
    }

    void visit(Symbol const& sym) override
    {
        if (sym.name() == MLL_QUOTE) {
            static auto quote_proc = make_proc([](List const& args, Env&) {
                return car(args);
            });
            result_ = quote_proc;
        }
        else {
            auto value = env_.lookup(sym.name());
            if (!value.has_value()) {
                throw EvalError("Unknown symbol: " + sym.name());
            }
            result_ = *value;
        }
    }

    void visit(Proc const& proc) override
    {
        assert(false);
    }

private:
    Env& env_;
    Node result_;
};
}

Node eval(Node expr, Env& env)
{
    return Evaluator{env}.evaluate(expr);
}
} // namespace mll