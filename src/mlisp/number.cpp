#include "number.hpp"

#include <cassert>
#include <iomanip>
#include <sstream>

namespace mlisp {

void NumberPrinter::print(std::ostream& ostream, mll::PrintContext /*context*/, double value)
{
    std::ostringstream oss;
    oss << std::fixed << value;

    auto str = oss.str();

    auto const dot_pos = str.find('.');
    assert(dot_pos != 0);
    assert(dot_pos != std::string::npos);

    auto const last_not_0_pos = str.find_last_not_of('0');
    if (last_not_0_pos == dot_pos) {
        str.resize(last_not_0_pos);
    }
    else if (last_not_0_pos != std::string::npos) {
        str.resize(last_not_0_pos + 1);
    }

    ostream << str;
}

} // namespace mlisp