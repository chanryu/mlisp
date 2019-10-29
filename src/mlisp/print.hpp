#pragma once

#include <mll.hpp>

inline std::ostream& operator << (std::ostream& os, mll::Node const& node)
{
    mll::BasicPrinter{os}.print(node);
    return os;
}

