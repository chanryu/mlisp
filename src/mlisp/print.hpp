#ifndef __MLISP_PRINT_HPP__
#define __MLISP_PRINT_HPP__

#include <mll.hpp>

inline std::ostream& operator << (std::ostream& os, mll::Node const& node)
{
    mll::BasicPrinter{os}.print(node);
    return os;
}

#endif
