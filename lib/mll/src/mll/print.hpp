#pragma once

#include <ostream>

namespace mll {

class Node;

enum class PrintContext {
    inspect, display
};

void print(std::ostream&, Node const&, PrintContext = PrintContext::inspect);

} // namespace mll

namespace std {
string to_string(mll::Node const& node);
} // namespace std
