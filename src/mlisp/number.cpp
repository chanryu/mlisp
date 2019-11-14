#include "number.hpp"

#include <cmath>
#include <iomanip>

namespace mlisp {

///////////////////////////////////////////////////////////////////////////////
// Number::Data

Number::Data::Data(double v) : value{v}
{}

void Number::Data::print(std::ostream& ostream, mll::PrintContext /*context*/)
{
    const bool is_integral = [this] {
        double int_part;
        std::modf(value, &int_part);
        return value == int_part;
    }();
    if (is_integral) {
        ostream << std::setprecision(0);
    }

    ostream << std::fixed << value;
}

///////////////////////////////////////////////////////////////////////////////
// Number

Number::Number(double value) : mll::Custom{std::make_shared<Data>(value)}
{}

Number::Number(Number const& other) : mll::Custom{other}
{}

Number::Number(std::shared_ptr<Data> const& data) : mll::Custom{data}
{}

double Number::value() const
{
    return static_cast<Data*>(Custom::data().get())->value;
}

std::optional<Number> Number::from_node(mll::Node const& node)
{
    if (auto data = std::dynamic_pointer_cast<Data>(node.data())) {
        return Number{data};
    }
    return std::nullopt;
}

} // namespace mlisp