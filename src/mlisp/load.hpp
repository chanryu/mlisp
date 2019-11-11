#pragma once

#include <string>

namespace mll {
class Env;
}

namespace mlisp {

bool load_file(mll::Env& env, std::string const& filepath);

} // namespace mlisp
