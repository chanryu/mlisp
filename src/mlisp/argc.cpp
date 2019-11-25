#include "argc.hpp"

#include <mll/eval.hpp>
#include <mll/list.hpp>

#include <string>

using namespace std::string_literals;

namespace mlisp {

void assert_argc(mll::List const& args, size_t count, char const* cmd)
{
    if (mll::length(args) != count) {
        throw mll::EvalError(cmd + " expects "s + std::to_string(count) + " argument(s).");
    }
}

void assert_argc_min(mll::List const& args, size_t min, char const* cmd)
{
    auto len = mll::length(args);
    if (len < min) {
        throw mll::EvalError(cmd + " expects "s + std::to_string(min) + " or more arguments.");
    }
}

} // namespace mlisp