#ifndef __MLISP_PRINT_HPP__
#define __MLISP_PRINT_HPP__

#include <mll.hpp>

class PrettyPrinter: public mll::BasicPrinter {
public:
    using BasicPrinter::BasicPrinter;
};

inline std::ostream& operator << (std::ostream& os, mll::Node const& node)
{
    PrettyPrinter{os}.print(node);
    return os;
}

#endif //__MLISP_PRINT_HPP__
