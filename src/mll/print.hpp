#pragma once

#include <ostream>

namespace mll {

class Node;

enum class StringStyle {
    quoted, raw
};

void print(std::ostream& ostream, Node const& node, StringStyle string_style = StringStyle::quoted);

} // namespace mll