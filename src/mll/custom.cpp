#include <mll/custom.hpp>

namespace mll {

void Custom::Data::accept(NodeVisitor& visitor)
{
    visitor.visit(Custom{std::static_pointer_cast<Data>(shared_from_this())});
}

Custom::Custom(std::shared_ptr<Data> data) : data_{data}
{}

std::shared_ptr<Custom::Data> const& Custom::data() const
{
    return data_;
}

} // namespace mll