#pragma once

#include <stdexcept>

namespace mll {

class Env;
class Node;

class EvalError: public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

Node eval(Node const& expr, Env& env); // throws EvalError

}