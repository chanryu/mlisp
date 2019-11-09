#pragma once

#include <mll/custom.hpp>

#include <ostream>

namespace mlisp {

class Fixnum : public mll::Custom {
public:
    explicit Fixnum(long);
    Fixnum(Fixnum const&);

    long value() const;

    static std::optional<Fixnum> from_node(mll::Node const&);

    struct Data : mll::Custom::Data {
        explicit Data(long v) : value{v}
        {}

        void print(std::ostream& ostream) override
        {
            ostream << value;
        }

        long const value;
    };
    std::shared_ptr<Data> const& data() const;

private:
    Fixnum(std::shared_ptr<Data> const&);
};

} // namespace mlisp