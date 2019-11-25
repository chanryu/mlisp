#include <mll/list.hpp>
#include <mll/symbol.hpp>

namespace mlisp {

inline mll::Node to_node(bool value)
{
    return value ? mll::Symbol{"t"} : mll::Node{};
}

inline bool to_bool(mll::Node const& node)
{
    auto list = mll::dynamic_node_cast<mll::List>(node);
    return !list || !list->empty();
}

} // namespace mlisp