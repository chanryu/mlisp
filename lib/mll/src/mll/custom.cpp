#include <mll/custom.hpp>

namespace mll {

void Custom::Core::accept(NodeVisitor& visitor)
{
    visitor.visit(Custom{std::static_pointer_cast<Core>(shared_from_this())});
}

Custom::Custom(Custom const& other) : _core{other._core}
{}

Custom::Custom(std::shared_ptr<Core> const& core) : _core{core}
{}

std::shared_ptr<Custom::Core> const& Custom::core() const
{
    return _core;
}

} // namespace mll