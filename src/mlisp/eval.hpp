#pragma once

#include <mll/mll.hpp>

namespace mlisp {
    
int eval_file(std::shared_ptr<mll::Env> env, const char* filename);
bool eval_stream(std::shared_ptr<mll::Env> env, std::istream& is, std::ostream& os);

} // namespace mlisp
