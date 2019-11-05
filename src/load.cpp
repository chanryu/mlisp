#include "load.hpp"

#include <mll/eval.hpp>
#include <mll/node.hpp>
#include <mll/parser.hpp>
#include <mll/print.hpp>

#include <fstream>

namespace mlisp {

bool load_file(mll::Env& env, const char* filename)
{
    if (std::ifstream ifs{filename}; ifs.is_open()) {
        mll::Parser parser;
        while (true) {
            auto expr = parser.parse(ifs);
            if (!expr.has_value()) {
                break;
            }
            eval(*expr, env);
        }
        return ifs.eof();
    }
    return false;
}

bool load_file(mll::Env& env, std::string const& filename)
{
    return load_file(env, filename.c_str());
}

} // namespace mlisp