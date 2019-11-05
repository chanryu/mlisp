#pragma once

#include <iostream>
#include <optional>

namespace mll {
class Env;
}

namespace mlisp {
    
bool load_file(mll::Env& env, const char* filename);
bool load_file(mll::Env& env, std::string const& filename);

} // namespace mlisp
