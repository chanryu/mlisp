#pragma once

#include <ostream>

namespace mll {

class Node;

struct PrintOptions {
    bool quote_string = true;
};

void print(std::ostream&, Node const&, PrintOptions const& = {});

} // namespace mll

namespace std {
string to_string(mll::Node const& node);
} // namespace std
