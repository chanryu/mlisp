#include <mll/custom.hpp>

namespace mll {

void Custom::Data::accept(NodeVisitor& visitor)
{
    visitor.visit(Custom{std::static_pointer_cast<Data>(shared_from_this())});
}

Custom::Custom(Custom const& other) : _data{other._data}
{}

Custom::Custom(std::shared_ptr<Data> const& data) : _data{data}
{}

std::shared_ptr<Custom::Data> const& Custom::data() const
{
    return _data;
}

} // namespace mll