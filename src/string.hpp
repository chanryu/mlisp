#pragma once

#include <mll/custom.hpp>

#include <string>

namespace mlisp {

class String : public mll::Custom {
public:
    explicit String(std::string);
    String(String const&);

    std::string const& value() const;

    struct Data : mll::Custom::Data {
        explicit Data(std::string v);
        void print(std::ostream&, mll::PrintOptions const&) override;
        std::string const value;
    };

    static std::optional<String> from_node(mll::Node const&);

private:
    String(std::shared_ptr<Data> const&);
};

} // namespace mlisp