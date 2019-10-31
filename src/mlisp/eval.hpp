#pragma once

#include <iostream>

namespace mll {
class Env;
}

namespace mlisp {
    
int eval_file(mll::Env& env, const char* filename);
bool eval_stream(mll::Env& env, std::istream& is, std::ostream& os);

} // namespace mlisp
