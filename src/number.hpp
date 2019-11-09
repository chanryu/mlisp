#pragma once

#include <mll/custom.hpp>

#include <ostream>

namespace mlisp {

class Number : public mll::Custom {
public:
    explicit Number(double);
    Number(Number const&);

    double value() const;

    struct Data : mll::Custom::Data {
        explicit Data(double v);
        void print(std::ostream&, mll::PrintOptions const&) override;
        double const value;
    };

    static std::optional<Number> from_node(mll::Node const&);

private:
    Number(std::shared_ptr<Data> const&);
};

} // namespace mlisp