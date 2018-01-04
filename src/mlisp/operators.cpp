#include <cassert>

#include "operators.hpp"
#include "eval.hpp"

#define MLISP_DEFUN(cmd__, proc)\
        do {\
            char const* cmd = cmd__;\
            env->set(cmd, proc);\
        } while (0)


using namespace mll;
using namespace std::string_literals;

inline bool is_number(Node const& node)
{
    return !!to_number(node);
}

inline bool is_string(Node const& node)
{
    return !!to_string(node);
}

inline bool is_symbol(Node const& node)
{
    return !!to_symbol(node);
}

inline Node cadr(List const& list)
{
    return car(cdr(list));
}

inline Node bool_to_node(bool value)
{
    return value ? make_symbol("t") : Node{};
}

template <typename Func>
void for_each(List list, Func func)
{
    while (!list.empty()) {
        func(car(list));
        list = cdr(list);
    }
};

static size_t length(List list)
{
    size_t len = 0;
    while (!list.empty()) {
        len ++;
        list = cdr(list);
    }
    return len;
}

static void assert_argc(List args, size_t count, char const *cmd)
{
    if (length(args) != count) {
        throw EvalError(cmd + " expects "s + std::to_string(count) + " argument(s).");
    }
}

static void assert_argc_min(List const& args, size_t min, char const *cmd)
{
    auto len = length(args);
    if (len < min) {
        throw EvalError(cmd + " expects "s + std::to_string(min) + " or more arguments.");
    }
}

static void assert_argc_range(List const& args, size_t min, size_t max, char const *cmd)
{
    auto len = length(args);

    if (len >= min && len <= max) {
        throw EvalError(cmd + " expects "s + std::to_string(min) + " ~ " +
                        std::to_string(max) + " argument(s).");
    }
}

static List to_list_or_throw(Node const& node, char const* cmd)
{
    auto list = to_list(node);
    if (!list) {
        throw EvalError(cmd + (": " + std::to_string(node)) + " is not a list.");
    }
    return *list;
}

static Number to_number_or_throw(Node const& node, char const* cmd)
{
    auto num = to_number(node);
    if (!num) {
        throw EvalError(cmd + (": " + std::to_string(node)) + " is not a number.");
    }
    return *num;
}

static String to_string_or_throw(Node const& node, char const* cmd)
{
    auto str = to_string(node);
    if (!str) {
        throw EvalError(cmd + (": " + std::to_string(node)) + " is not a string.");
    }
    return *str;
}

static Symbol to_symbol_or_throw(Node const& node, char const* cmd)
{
    auto sym = to_symbol(node);
    if (!sym) {
        throw EvalError(cmd + (": " + std::to_string(node)) + " is not a symbol.");
    }
    return *sym;
}

static List to_formal_args(Node const& node, char const* cmd)
{
    auto args = to_list_or_throw(node, cmd);

    // validate args (must be list of symbols)
    for (auto c = args; !c.empty(); c = cdr(c)) {
        if (!is_symbol(car(c))) {
            throw EvalError(cmd + (": " + std::to_string(car(c))) + " is not a symbol");
        }
    }

    return args;
}

void set_primitive_operators(std::shared_ptr<Env> env)
{
    // "quote" is already built into the Parser/eval()
    //env->set("quote", make_proc([](List args, Env) {
    //    return car(args);
    //}));

    MLISP_DEFUN("atom", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc(args, 1, cmd);
        auto list = to_list(eval(car(args), env));
        return bool_to_node(!list || list->empty());
    }));

    MLISP_DEFUN("eq", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc(args, 2, cmd);

        auto lhs = eval(car(args), env);
        auto rhs = eval(cadr(args), env);
        return bool_to_node(lhs.data() == rhs.data());
    }));

    MLISP_DEFUN("car", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc(args, 1, cmd);
        return car(to_list_or_throw(eval(car(args), env), cmd));
    }));

    MLISP_DEFUN("cdr", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc(args, 1, cmd);
        return cdr(to_list_or_throw(eval(car(args), env), cmd));
    }));

    MLISP_DEFUN("cons", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc(args, 2, cmd);
        auto head = eval(car(args), env);
        auto tail = to_list_or_throw(eval(cadr(args), env), cmd);

        return cons(head, tail);
    }));

    MLISP_DEFUN("cond", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        Node result;
        while (!args.empty()) {
            auto clause = to_list_or_throw(car(args), cmd);
            auto pred = car(clause);
            if (eval(pred, env)) {
                for (auto expr = cdr(clause); !expr.empty(); expr = cdr(expr)) {
                    result = eval(car(expr), env);
                }
                return result;
            }
            args = cdr(args);
        }
        return result;
    }));

    MLISP_DEFUN("lambda", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc_min(args, 2, cmd);

        auto formal_args = to_formal_args(car(args), cmd);
        auto lambda_body = cadr(args);
        auto creator_env = env;

        return make_proc([formal_args, lambda_body, creator_env] (List args, std::shared_ptr<Env> env) {
            auto lambda_env = creator_env->derive_new();

            auto syms = formal_args;
            while (!syms.empty()) {
                if (args.empty()) {
                    EvalError("Proc: too few args");
                }

                assert(is_symbol(car(syms)));

                auto sym = to_symbol(car(syms));
                auto val = eval(car(args), env);
                lambda_env->set(sym->name(), val);
                syms = cdr(syms);
                args = cdr(args);
            }

            if (!args.empty()) {
                EvalError("Proc: too many args");
            }

            return eval(lambda_body, lambda_env);
        });
    }));

    MLISP_DEFUN("label", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc(args, 2, cmd);

        auto symbol = to_symbol_or_throw(car(args), cmd);
        auto value = eval(cadr(args), env);
        env->set(symbol.name(), value);

        return value;
    }));

    MLISP_DEFUN("defun", make_proc([] (List args, std::shared_ptr<Env> env) {
        auto name = car(args);
        auto body = cons(make_symbol("lambda"), cdr(args));
        return eval(cons(make_symbol("label"), cons(name, cons(body, {}))), env);
    }));
}

void set_complementary_operators(std::shared_ptr<Env> env)
{
    MLISP_DEFUN("list", make_proc([] (List args, std::shared_ptr<Env> env) {
        std::vector<Node> objs;
        for_each(args, [&objs, env] (auto const& arg) {
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
    }));

    MLISP_DEFUN("set", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc(args, 2, cmd);

        auto symbol = to_symbol_or_throw(eval(car(args), env), cmd);
        auto value = eval(cadr(args), env);
        env->set(symbol.name(), value);
        return value;
    }));

    MLISP_DEFUN("if", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc_range(args, 2, 3, cmd);

        auto cond = car(args);
        auto body = cdr(args);
        auto then_arm = car(body);
        auto else_arm = cadr(body);
        if (eval(cond, env)) {
            return eval(then_arm, env);
        }
        return eval(else_arm, env);
    }));

    MLISP_DEFUN("print", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        Node result;
        auto first = true;
        for_each(args, [&first, env, &result] (auto const& arg) {
            if (first) {
                first = false;
            }
            else {
                std::cout << " ";
            }
            result = eval(arg, env);
            std::cout << result;
        });
        std::cout << std::endl;
        return result;
    }));

    MLISP_DEFUN("load", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc(args, 1, cmd);
        auto filename = to_string_or_throw(car(args), cmd);
        return bool_to_node(!eval_file(env, filename.text().c_str()));
    }));
}

void set_number_operators(std::shared_ptr<Env> env)
{
    MLISP_DEFUN("number?", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc(args, 1, cmd);
        return bool_to_node(is_number(eval(car(args), env)));
    }));

    MLISP_DEFUN("number-equal?", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc(args, 2, cmd);

        auto num1 = to_number_or_throw(eval(car(args), env), "number-equal?");
        auto num2 = to_number_or_throw(eval(cadr(args), env), "number-equal?");

        return bool_to_node(num1.value() == num2.value());
    }));

    MLISP_DEFUN("number-less?", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc(args, 2, cmd);

        auto num1 = to_number_or_throw(eval(car(args), env), cmd);
        auto num2 = to_number_or_throw(eval(cadr(args), env), cmd);
        return bool_to_node(num1.value() < num2.value());
    }));

    MLISP_DEFUN("+", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        auto result = 0.0;
        for_each(args, [&result, env, cmd](auto const& arg) {
            result += to_number_or_throw(eval(arg, env), cmd).value();
        });
        return make_number(result);
    }));

    MLISP_DEFUN("-", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc_min(args, 1, cmd);

        auto result = to_number_or_throw(eval(car(args), env), cmd).value();
        args = cdr(args);
        if (args.empty()) {
            // unary minus
            result = -result;
        }
        else {
            for_each(args, [&result, env, cmd](auto const& arg) {
                result -= to_number_or_throw(eval(arg, env), cmd).value();
            });
        }

        return make_number(result);
    }));

    MLISP_DEFUN("*", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        auto result = 1.0;
        while (!args.empty()) {
            auto arg = eval(car(args), env);
            result *= to_number_or_throw(arg, cmd).value();
            args = cdr(args);
        }

        return make_number(result);
    }));

    MLISP_DEFUN("/", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc_min(args, 2, cmd);

        auto result = to_number_or_throw(eval(car(args), env), cmd).value();
        for_each(cdr(args), [&result, cmd, env](auto const& arg) {
            result /= to_number_or_throw(eval(arg, env), cmd).value();
        });
        return make_number(result);
    }));
}

void set_string_operators(std::shared_ptr<Env> env)
{
    MLISP_DEFUN("string?", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc(args, 1, cmd);
        return bool_to_node(is_string(eval(car(args), env)));
    }));

    MLISP_DEFUN("string-equal?", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc(args, 2, cmd);

        auto str1 = to_string_or_throw(eval(car(args), env), cmd);
        auto str2 = to_string_or_throw(eval(cadr(args), env), cmd);

        return bool_to_node(str1.text() == str2.text());
    }));
}

void set_symbol_operators(std::shared_ptr<Env> env)
{
    MLISP_DEFUN("symbol?", make_proc([cmd] (List args, std::shared_ptr<Env> env) {
        assert_argc(args, 1, cmd);
        return bool_to_node(is_symbol(eval(car(args), env)));
    }));
}
