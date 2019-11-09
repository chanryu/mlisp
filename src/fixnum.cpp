#include "fixnum.hpp"

namespace mlisp {

Fixnum::Fixnum(long value) : mll::Custom{std::make_shared<Data>(value)}
{}

Fixnum::Fixnum(Fixnum const& other) : mll::Custom{other}
{}

Fixnum::Fixnum(std::shared_ptr<Data> const& data) : mll::Custom{data}
{}

long Fixnum::value() const
{
    return data()->value;
}

std::optional<Fixnum> Fixnum::from_node(mll::Node const& node)
{
    if (auto data = std::dynamic_pointer_cast<Data>(node.data())) {
        return Fixnum{data};
    }
    return std::nullopt;
}

std::shared_ptr<Fixnum::Data> const& Fixnum::data() const
{
    return std::static_pointer_cast<Fixnum::Data>(Custom::data());
}

} // namespace mlisp