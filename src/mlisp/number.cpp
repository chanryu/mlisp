#include "number.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace mlisp {
MLL_CUSTOM_TYPE_IMPL(Number, double, [](auto& ostream, auto /*context*/, auto value) {
    auto str = (std::ostringstream{} << std::fixed << value).str();

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
});
} // namespace mlisp