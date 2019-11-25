#include "operators.hpp"

#include <mll/env.hpp>
#include <mll/eval.hpp>
#include <mll/list.hpp>
#include <mll/print.hpp>
#include <mll/proc.hpp>
#include <mll/quote.hpp>
#include <mll/symbol.hpp>

#include <cassert>
#include <iostream>
#include <sstream>
#include <stack>
#include <string_view>
#include <vector>

#include "argc.hpp"
#include "bool.hpp"
#include "load.hpp"
#include "string.hpp"

#define MLISP_DEFUN(cmd__, func__)                                                                                     \
    do {                                                                                                               \
        auto const cmd = cmd__;                                                                                        \
        env.set(cmd, Proc{cmd, func__});                                                                               \
    } while (0)

using namespace mll;

namespace mlisp {

namespace {

bool is_symbol(Node const& node)
{
    return dynamic_node_cast<Symbol>(node).has_value();
}

} // namespace

void set_complementary_procs(Env& env)
{
    MLISP_DEFUN("print", [/*cmd*/](List const& args, Env& env) {
        for_each_with_index(args, [&env](auto const i, auto const& expr) {
            if (i != 0) {
                std::cout << ' ';
            }
            print(std::cout, eval(expr, env), PrintContext::display);
        });
        std::cout << std::endl;
        return nil;
    });

    MLISP_DEFUN("load", [cmd](List args, Env& env) {
        assert_argc(args, 1, cmd);
        auto filename = dynamic_node_cast<String>(car(args));
        if (!filename) {
            throw EvalError("load: " + std::to_string(car(args)) + " does not evaluate to a string.");
        }
        return to_node(!load_file(env, filename->value()));
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