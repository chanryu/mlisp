#pragma once

#include <mll/custom.hpp>

#include <string>

namespace mll {
class Env;
}

namespace mlisp {

struct StringPrinter {
    static void print(std::ostream&, mll::PrintContext, std::string const&);
};

using String = mll::CustomType<std::string, StringPrinter>;

void set_string_procs(mll::Env& env);

} // namespace mlisp