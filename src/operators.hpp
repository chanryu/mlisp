#pragma once

namespace mll {
class Env;
}

namespace mlisp {

void set_quote_procs(mll::Env& env);
void set_primitive_procs(mll::Env& env);
void set_complementary_procs(mll::Env& env);

void set_number_procs(mll::Env& env);
void set_string_procs(mll::Env& env);
void set_symbol_procs(mll::Env& env);

} // namespace mlisp