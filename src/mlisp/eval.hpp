#ifndef __MLISP_EVAL_HPP__
#define __MLISP_EVAL_HPP__

#include <mll.hpp>

int eval_file(std::shared_ptr<mll::Env> env, const char* filename);
bool eval_stream(std::shared_ptr<mll::Env> env, std::istream& is, std::ostream& os);

#endif //__MLISP_EVAL_HPP__
