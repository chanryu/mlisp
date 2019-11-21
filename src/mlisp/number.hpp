#pragma once

#include <mll/custom.hpp>

namespace mlisp {

struct NumberPrinter {
    static void print(std::ostream&, mll::PrintContext, double);
};

using Number = mll::CustomType<double, NumberPrinter>;

} // namespace mlisp