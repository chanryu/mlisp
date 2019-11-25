#pragma once

#include <mll/custom.hpp>

namespace mll {
class Env;
}

namespace mlisp {

struct NumberPrinter {
    static void print(std::ostream&, mll::PrintContext, double);
};

using Number = mll::CustomType<double, NumberPrinter>;

void set_number_procs(mll::Env& env);

} // namespace mlisp