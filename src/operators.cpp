#include "operators.hpp"

#include <mll/env.hpp>
#include <mll/eval.hpp>
#include <mll/node.hpp>
#include <mll/print.hpp>

#include <cassert>
#include <sstream>
#include <stack>
#include <vector>

#include "load.hpp"

#define MLISP_DEFUN(cmd__, func__)                                                                                     \
    do {                                                                                                               \
        char const* cmd = cmd__;                                                                                       \
        env.set(cmd, make_proc(func__));                                                                               \
    } while (0)

using namespace mll;
using namespace std::string_literals;

namespace std {
string to_string(Node const& node)
{
    ostringstream ss;
    print(ss, node);
    return ss.str();
}
} // namespace std

namespace mlisp {

namespace {

inline Proc make_proc(Func func)
{
    return Proc{std::move(func)};
}

bool is_number(Node const& node)
{
    return dynamic_node_cast<Number>(node).has_value();
}

bool is_string(Node const& node)
{
    return dynamic_node_cast<String>(node).has_value();
}

bool is_symbol(Node const& node)
{
    return dynamic_node_cast<Symbol>(node).has_value();
}

Node cadr(List const& list)
{
    return car(cdr(list));
}

bool to_bool(Node const& node)
{
    auto list = dynamic_node_cast<List>(node);
    if (list && list->empty()) {
        return false;
    }
    return true;
}

Node to_node(bool value)
{
    return value ? Symbol{"t"} : Node{};
}

template <typename Func>
void for_each(List list, Func const& func)
{
    while (!list.empty()) {
        func(car(list));
        list = cdr(list);
    }
}

template <typename Func>
void for_each_with_index(List list, Func const& func)
{
    for (size_t i = 0; !list.empty(); ++i) {
        func(i, car(list));
        list = cdr(list);
    }
}

size_t length(List list)
{
    size_t len = 0;
    while (!list.empty()) {
        len++;
        list = cdr(list);
    }
    return len;
}

void assert_argc(List const& args, size_t count, char const* cmd)
{
    if (length(args) != count) {
        throw EvalError(cmd + " expects "s + std::to_string(count) + " argument(s).");
    }
}

void assert_argc_min(List const& args, size_t min, char const* cmd)
{
    auto len = length(args);
    if (len < min) {
        throw EvalError(cmd + " expects "s + std::to_string(min) + " or more arguments.");
    }
}

void assert_argc_range(List const& args, size_t min, size_t max, char const* cmd)
{
    auto len = length(args);
    if (len > min || len < max) {
        throw EvalError(cmd + " expects "s + std::to_string(min) + " ~ " + std::to_string(max) + " argument(s).");
    }
}

List to_list_or_throw(Node const& node, char const* cmd)
{
    auto list = dynamic_node_cast<List>(node);
    if (!list) {
        throw EvalError(cmd + (": " + std::to_string(node)) + " is not a list.");
    }
    return *list;
}

Number to_number_or_throw(Node const& node, char const* cmd)
{
    auto num = dynamic_node_cast<Number>(node);
    if (!num) {
        throw EvalError(cmd + (": " + std::to_string(node)) + " is not a number.");
    }
    return *num;
}

String to_string_or_throw(Node const& node, char const* cmd)
{
    auto str = dynamic_node_cast<String>(node);
    if (!str) {
        throw EvalError(cmd + (": " + std::to_string(node)) + " is not a string.");
    }
    return *str;
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

List to_formal_args(Node const& node, char const* cmd)
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
} // namespace

void set_primitive_procs(Env& env)
{
    // "quote" is already built into the Parser/eval()
    // env->set("quote", [](List args, Env) {
    //    return car(args);
    //});

    MLISP_DEFUN("atom", [cmd](List args, Env& env) {
        assert_argc(args, 1, cmd);
        auto list = dynamic_node_cast<List>(eval(car(args), env));
        return to_node(!list || list->empty());
    });

    MLISP_DEFUN("eq", [cmd](List args, Env& env) {
        assert_argc(args, 2, cmd);
        auto lhs = eval(car(args), env);
        auto rhs = eval(cadr(args), env);
        return to_node(lhs.data() == rhs.data());
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
            auto pred = car(clause);
            if (to_bool(eval(pred, env))) {
                for (auto expr = cdr(clause); !expr.empty(); expr = cdr(expr)) {
                    result = eval(car(expr), env);
                }
                return result;
            }
            args = cdr(args);
        }
        return result;
    });

    MLISP_DEFUN("lambda", [cmd](List args, Env& env) {
        assert_argc_min(args, 2, cmd);

        auto formal_args = to_formal_args(car(args), cmd);
        auto lambda_body = cadr(args);
        auto creator_env = env.shared_from_this();

        return make_proc([formal_args, lambda_body, creator_env](List args, Env& env) {
            auto lambda_env = creator_env->derive_new();
            auto syms = formal_args;
            while (!syms.empty()) {
                auto sym = dynamic_node_cast<Symbol>(car(syms));
                assert(sym.has_value());

                if (is_variadic_args(*sym)) {
                    std::stack<Node> args_stack;
                    while (!args.empty()) {
                        args_stack.push(eval(car(args), env));
                        args = cdr(args);
                    }
                    List vargs;
                    while (!args_stack.empty()) {
                        vargs = cons(args_stack.top(), vargs);
                        args_stack.pop();
                    }
                    lambda_env->set(sym->name().substr(1), vargs);
                    break;
                }

                if (args.empty()) {
                    EvalError("Proc: too few args");
                }

                auto val = eval(car(args), env);
                lambda_env->set(sym->name(), val);
                syms = cdr(syms);
                args = cdr(args);
            }

            if (!args.empty()) {
                EvalError("Proc: too many args");
            }

            return eval(lambda_body, *lambda_env);
        });
    });

    MLISP_DEFUN("defun", [](List args, Env& env) {
        auto name = car(args);
        auto body = cons(Symbol{"lambda"}, cdr(args));
        return eval(cons(Symbol{"define"}, cons(name, cons(body, {}))), env);
    });
}

void set_complementary_procs(Env& env)
{

    MLISP_DEFUN("list", [](List args, Env& env) {
        std::vector<Node> objs;
        for_each(args, [&objs, &env](auto const& arg) {
            objs.push_back(eval(arg, env));
        });
        while (!args.empty()) {
            args = cdr(args);
        }

        List list;
        while (!objs.empty()) {
            list = cons(objs.back(), list);
            objs.pop_back();
        }
        return list;
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
        if (!env.update(symbol.name(), value)) {
            throw EvalError("unbound variable: " + symbol.name());
        }
        return value;
    });

    /*
    MLISP_DEFUN("begin", [/](List args, Env& env) {
        auto new_env = env.derive_new();
        Node value;
        while (!args.empty()) {
            value = eval(car(args), *new_env);
            args = cdr(args);
        }
        return value;
    });
    */

    MLISP_DEFUN("if", [cmd](List args, Env& env) {
        assert_argc_range(args, 2, 3, cmd);

        auto cond = car(args);
        auto body = cdr(args);
        auto then_arm = car(body);
        auto else_arm = cadr(body);
        if (to_bool(eval(cond, env))) {
            return eval(then_arm, env);
        }
        return eval(else_arm, env);
    });

    MLISP_DEFUN("print", [/*cmd*/](List const& args, Env& env) {
        for_each_with_index(args, [&env](auto const i, auto const& expr) {
            if (i != 0) {
                std::cout << ' ';
            }
            print(std::cout, eval(expr, env), StringStyle::raw);
        });
        std::cout << '\n';
        return nil;
    });

    MLISP_DEFUN("load", [cmd](List args, Env& env) {
        assert_argc(args, 1, cmd);
        auto filename = to_string_or_throw(car(args), cmd);
        return to_node(!load_file(env, filename.text()));
    });
}

void set_number_procs(Env& env)
{

    MLISP_DEFUN("number?", [cmd](List args, Env& env) {
        assert_argc(args, 1, cmd);
        return to_node(is_number(eval(car(args), env)));
    });

    MLISP_DEFUN("number-equal?", [cmd](List args, Env& env) {
        assert_argc(args, 2, cmd);

        auto num1 = to_number_or_throw(eval(car(args), env), cmd);
        auto num2 = to_number_or_throw(eval(cadr(args), env), cmd);

        return to_node(num1.value() == num2.value());
    });

    MLISP_DEFUN("number-less?", [cmd](List args, Env& env) {
        assert_argc(args, 2, cmd);

        auto num1 = to_number_or_throw(eval(car(args), env), cmd);
        auto num2 = to_number_or_throw(eval(cadr(args), env), cmd);
        return to_node(num1.value() < num2.value());
    });

    MLISP_DEFUN("+", [cmd](List args, Env& env) {
        auto result = 0.0;
        for_each(args, [&result, &env, cmd](auto const& arg) {
            result += to_number_or_throw(eval(arg, env), cmd).value();
        });
        return Number{result};
    });

    MLISP_DEFUN("-", [cmd](List args, Env& env) {
        assert_argc_min(args, 1, cmd);

        auto result = to_number_or_throw(eval(car(args), env), cmd).value();
        args = cdr(args);
        if (args.empty()) {
            // unary minus
            result = -result;
        }
        else {
            for_each(args, [&result, &env, cmd](auto const& arg) {
                result -= to_number_or_throw(eval(arg, env), cmd).value();
            });
        }
        return Number{result};
    });

    MLISP_DEFUN("*", [cmd](List args, Env& env) {
        auto result = 1.0;
        while (!args.empty()) {
            auto arg = eval(car(args), env);
            result *= to_number_or_throw(arg, cmd).value();
            args = cdr(args);
        }
        return Number{result};
    });

    MLISP_DEFUN("/", [cmd](List args, Env& env) {
        assert_argc_min(args, 2, cmd);

        auto result = to_number_or_throw(eval(car(args), env), cmd).value();
        for_each(cdr(args), [&result, &env, cmd](auto const& arg) {
            result /= to_number_or_throw(eval(arg, env), cmd).value();
        });
        return Number{result};
    });
}

void set_string_procs(Env& env)
{
    MLISP_DEFUN("string?", [cmd](List const& args, Env& env) {
        assert_argc(args, 1, cmd);
        return to_node(is_string(eval(car(args), env)));
    });

    MLISP_DEFUN("string-equal?", [cmd](List const& args, Env& env) {
        assert_argc(args, 2, cmd);

        auto const str1 = to_string_or_throw(eval(car(args), env), cmd);
        auto const str2 = to_string_or_throw(eval(cadr(args), env), cmd);
        return to_node(str1.text() == str2.text());
    });
}

void set_symbol_procs(Env& env)
{
    MLISP_DEFUN("symbol?", [cmd](List const& args, Env& env) {
        assert_argc(args, 1, cmd);
        return to_node(is_symbol(eval(car(args), env)));
    });
}

} // namespace mlisp