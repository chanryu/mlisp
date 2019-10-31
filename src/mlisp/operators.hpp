#pragma once

#include <mll/mll.hpp>

namespace mlisp {

void set_primitive_procs(std::shared_ptr<mll::Env> env);
void set_complementary_procs(std::shared_ptr<mll::Env> env);

void set_number_procs(std::shared_ptr<mll::Env> env);
void set_string_procs(std::shared_ptr<mll::Env> env);
void set_symbol_procs(std::shared_ptr<mll::Env> env);

} // namespace mlisp