#pragma once

#include <mll/custom.hpp>

#include <string>

namespace mlisp {

struct StringPrinter {
    StringPrinter(std::ostream&, mll::PrintContext, std::string const&);
};

using String = mll::CustomType<std::string, StringPrinter>;

} // namespace mlisp