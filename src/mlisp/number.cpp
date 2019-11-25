#include "number.hpp"

#include "argc.hpp"
#include "bool.hpp"

#include <mll/env.hpp>
#include <mll/eval.hpp>
#include <mll/list.hpp>
#include <mll/print.hpp>
#include <mll/proc.hpp>

#include <cassert>
#include <iomanip>
#include <sstream>

#define MLISP_DEFUN(cmd__, func__)                                                                                     \
    do {                                                                                                               \
        auto const cmd = cmd__;                                                                                        \
        env.set(cmd, Proc{cmd, func__});                                                                               \
    } while (0)

namespace mlisp {

namespace {
bool is_number(mll::Node const& node)
{
    return mll::dynamic_node_cast<Number>(node).has_value();
}

Number to_number_or_throw(mll::Node const& node, char const* cmd)
{
    auto num = mll::dynamic_node_cast<Number>(node);
    if (!num) {
        throw mll::EvalError(cmd + (": " + std::to_string(node)) + " is not a number.");
    }
    return *num;
}
} // namespace

void NumberPrinter::print(std::ostream& ostream, mll::PrintContext /*context*/, double value)
{
    std::ostringstream oss;
    oss << std::fixed << value;

    auto str = oss.str();

    auto const dot_pos = str.find('.');
    assert(dot_pos != 0);
    assert(dot_pos != std::string::npos);

    auto const last_not_0_pos = str.find_last_not_of('0');
    if (last_not_0_pos == dot_pos) {
        str.resize(last_not_0_pos);
    }
    else if (last_not_0_pos != std::string::npos) {
        str.resize(last_not_0_pos + 1);
    }

    ostream << str;
}

void set_number_procs(mll::Env& env)
{
    using namespace mll;

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
        for_each(args,
                 [&result, &env, cmd](auto const& arg) { result += to_number_or_throw(eval(arg, env), cmd).value(); });
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
        for_each(cdr(args),
                 [&result, &env, cmd](auto const& arg) { result /= to_number_or_throw(eval(arg, env), cmd).value(); });
        return Number{result};
    });
}

} // namespace mlisp