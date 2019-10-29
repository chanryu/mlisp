#include "eval.hpp"

#include <fstream>
#include <sstream>

#include "print.hpp"

bool
eval_stream(std::shared_ptr<mll::Env> env, std::istream& is, std::ostream& os)
{
    try {
        auto parser = mll::Parser{};

        while (true) {
            auto expr = parser.parse(is);
            if (!expr) {
                break;
            }
            os << eval(*expr, env) << std::endl;
        }
    }
    catch (mll::ParseError& e) {
        std::cout << e.what() << std::endl;
        return false;
    }
    catch (mll::EvalError& e) {
        std::cout << e.what() << std::endl;
        return false;
    }

    return is.eof();
}

int
eval_file(std::shared_ptr<mll::Env> env, const char* filename)
{
    auto ifs = std::ifstream{ filename };
    if (!ifs.is_open()) {
        return -1;
    }

    // throw-away stream
    std::ostringstream oss;
    oss.setstate(std::ios_base::badbit);

    return eval_stream(env, ifs, oss);
}
