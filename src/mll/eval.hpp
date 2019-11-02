#pragma once

#include <map>
#include <memory>
#include <string>

namespace mll {

class Env;
class Node;

struct EvalError: std::runtime_error {
    using runtime_error::runtime_error;
};

Node eval(Node expr, Env& env); // throws EvalError

}