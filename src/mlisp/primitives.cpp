#include "primitives.hpp"

#include <mll/env.hpp>
#include <mll/eval.hpp>
#include <mll/list.hpp>
#include <mll/print.hpp>
#include <mll/proc.hpp>
#include <mll/symbol.hpp>

#include <cassert>

#include "argc.hpp"
#include "bool.hpp"

#define MLISP_DEFUN(cmd__, func__)                                                                                     \
    do {                                                                                                               \
        auto const cmd = cmd__;                                                                                        \
        env.set(cmd, Proc{cmd, func__});                                                                               \
    } while (0)

using namespace mll;

namespace mlisp {

namespace {

List to_list_or_throw(Node const& node, char const* cmd)
{
    auto list = dynamic_node_cast<List>(node);
    if (!list) {
        throw EvalError(cmd + (": " + std::to_string(node)) + " is not a list.");
    }
    return *list;
}

Symbol to_symbol_or_throw(Node const& node, char const* cmd)
{
    auto sym = dynamic_node_cast<Symbol>(node);
    if (!sym) {
        throw EvalError(cmd + (": " + std::to_string(node)) + " is not a symbol.");
    }
    return *sym;
}

bool is_variadic_args(Symbol const& sym)
{
    return sym.name().size() > 1 && sym.name()[0] == '*';
}

List to_formal_args_or_throw(Node const& node, char const* cmd)
{
    auto args = to_list_or_throw(node, cmd);

    // validate args (must be list of symbols)
    for (auto c = args; !c.empty(); c = cdr(c)) {
        auto sym = dynamic_node_cast<Symbol>(car(c));
        if (!sym.has_value()) {
            throw EvalError(cmd + (": " + std::to_string(car(c))) + " is not a symbol");
        }
        if (is_variadic_args(*sym) && !cdr(c).empty()) {
            throw EvalError(cmd + (": " + sym->name()) + " must be the last argument");
        }
    }

    return args;
}

Node make_lambda(std::string name, List const& formal_args, List const& lambda_body,
                 std::shared_ptr<Env> const& outer_env)
{
    return Proc(std::move(name), [formal_args, lambda_body, outer_env](List args, Env& env) {
        auto lambda_env = outer_env->derive_new();
        auto syms = formal_args;
        while (!syms.empty()) {
            auto sym = dynamic_node_cast<Symbol>(car(syms));
            assert(sym.has_value());

            if (is_variadic_args(*sym)) {
                args = map(args, [&env](Node const& node) { return eval(node, env); });
                lambda_env->set(sym->name().substr(1), args);
                args = nil;
                break;
            }

            if (args.empty()) {
                throw EvalError("Proc: too few args");
            }

            auto val = eval(car(args), env);
            lambda_env->set(sym->name(), val);
            syms = cdr(syms);
            args = cdr(args);
        }

        if (!args.empty()) {
            throw EvalError("Proc: too many args");
        }

        Node result;
        for_each(lambda_body, [&result, lambda_env](auto const& expr) { result = eval(expr, *lambda_env); });

        return result;
    });
}

Proc make_macro(std::string name, List const& formal_args, Node const& macro_body)
{
    return Proc(std::move(name), [formal_args, macro_body](List args, Env& env) {
        auto macro_env = env.derive_new();
        auto syms = formal_args;
        while (!syms.empty()) {
            auto sym = dynamic_node_cast<Symbol>(car(syms));
            assert(sym.has_value());

            if (is_variadic_args(*sym)) {
                macro_env->set(sym->name().substr(1), args);
                args = nil;
                break;
            }

            if (args.empty()) {
                throw EvalError("Proc: too few args");
            }

            auto val = car(args);
            macro_env->set(sym->name(), val);
            syms = cdr(syms);
            args = cdr(args);
        }

        if (!args.empty()) {
            throw EvalError("Proc: too many args");
        }

        auto expr = eval(macro_body, *macro_env);
        return eval(expr, env);
    });
}

} // namespace

void set_primitive_procs(Env& env)
{
    MLISP_DEFUN("atom", [cmd](List args, Env& env) {
        assert_argc(args, 1, cmd);
        auto list = dynamic_node_cast<List>(eval(car(args), env));
        return to_node(!list || list->empty());
    });

    MLISP_DEFUN("eq", [cmd](List args, Env& env) {
        assert_argc(args, 2, cmd);
        auto lhs = eval(car(args), env);
        auto rhs = eval(cadr(args), env);
        return to_node(lhs.core() == rhs.core());
    });

    MLISP_DEFUN("car", [cmd](List args, Env& env) {
        assert_argc(args, 1, cmd);
        return car(to_list_or_throw(eval(car(args), env), cmd));
    });

    MLISP_DEFUN("cdr", [cmd](List args, Env& env) {
        assert_argc(args, 1, cmd);
        return cdr(to_list_or_throw(eval(car(args), env), cmd));
    });

    MLISP_DEFUN("cons", [cmd](List args, Env& env) {
        assert_argc(args, 2, cmd);
        auto head = eval(car(args), env);
        auto tail = to_list_or_throw(eval(cadr(args), env), cmd);
        return cons(head, tail);
    });

    MLISP_DEFUN("cond", [cmd](List args, Env& env) {
        Node result;
        while (!args.empty()) {
            auto clause = to_list_or_throw(car(args), cmd);
            if (to_bool(eval(car(clause), env))) {
                result = eval(cadr(clause), env);
                break;
            }
            args = cdr(args);
        }
        return result;
    });

    MLISP_DEFUN("define", [cmd](List args, Env& env) {
        assert_argc(args, 2, cmd);

        auto symbol = to_symbol_or_throw(car(args), cmd);
        auto value = eval(cadr(args), env);
        env.set(symbol.name(), value);
        return value;
    });

    MLISP_DEFUN("set!", [cmd](List args, Env& env) {
        assert_argc(args, 2, cmd);

        auto symbol = to_symbol_or_throw(car(args), cmd);
        auto value = eval(cadr(args), env);
        if (!env.deep_update(symbol.name(), value)) {
            throw EvalError("unbound variable: " + symbol.name());
        }
        return value;
    });

    MLISP_DEFUN("lambda", [cmd](List args, Env& env) {
        assert_argc_min(args, 2, cmd);

        auto formal_args = to_formal_args_or_throw(car(args), cmd);
        auto lambda_body = cdr(args);
        auto outer_env = env.shared_from_this();

        return make_lambda("anonymous", formal_args, lambda_body, outer_env);
    });

    MLISP_DEFUN("macro", [cmd](List args, Env& /*env*/) {
        assert_argc_min(args, 2, cmd);

        auto formal_args = to_formal_args_or_throw(car(args), cmd);
        auto macro_body = cadr(args);

        return make_macro("anonymous", formal_args, macro_body);
    });
}

} // namespace mlisp