#pragma once

namespace mll {
class Env;
}

namespace mlisp {

void set_complementary_procs(mll::Env& env);
void set_symbol_procs(mll::Env& env);

} // namespace mlisp