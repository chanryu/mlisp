#pragma once

#include <ostream>

namespace mll {

class Node;

enum class StringStyle { quoted, raw };

void print(std::ostream&, Node const&, StringStyle = StringStyle::quoted);

} // namespace mll