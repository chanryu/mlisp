#pragma once

#include <mll/custom.hpp>

namespace mlisp {

struct NumberPrinter {
    NumberPrinter(std::ostream&, mll::PrintContext, double);
};

using Number = mll::CustomType<double, NumberPrinter>;

} // namespace mlisp