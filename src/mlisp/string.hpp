#pragma once

#include <mll/custom.hpp>

#include <string>

namespace mlisp {

struct StringPrinter {
    static void print(std::ostream&, mll::PrintContext, std::string const&);
};

using String = mll::CustomType<std::string, StringPrinter>;

} // namespace mlisp