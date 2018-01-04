#ifndef __MLISP_OPERATORS_HPP__
#define __MLISP_OPERATORS_HPP__

#include <mll.hpp>

void set_primitive_procs(std::shared_ptr<mll::Env> env);
void set_complementary_procs(std::shared_ptr<mll::Env> env);

void set_number_procs(std::shared_ptr<mll::Env> env);
void set_string_procs(std::shared_ptr<mll::Env> env);
void set_symbol_procs(std::shared_ptr<mll::Env> env);

#endif //__MLISP_OPERATORS_HPP__
