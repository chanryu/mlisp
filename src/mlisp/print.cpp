#include "print.hpp"

void
PrettyPrinter::visit(mll::List list)
{
    if (!list) {
        ostream_ << "nil";
        return;
    }

    BasicPrinter::visit(list);
}
