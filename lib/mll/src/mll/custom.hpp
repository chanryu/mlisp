#pragma once

#include <mll/node.hpp>

#include <ostream>

namespace mll {

class Parser;
enum class PrintContext;

class Custom {
public:
    Custom(Custom const&);

    struct Data : Node::Data {
        void accept(NodeVisitor&) final;
        virtual void print(std::ostream&, PrintContext) = 0;
    };
    std::shared_ptr<Data> const& data() const;

protected:
    explicit Custom(std::shared_ptr<Data> const&);

private:
    friend class Parser;
    std::shared_ptr<Data> _data;
};

template <typename ValueType, typename PrintFunc>
class CustomType final : public ::mll::Custom {
public:
    struct Data : ::mll::Custom::Data {
        explicit Data(ValueType v) : value{std::move(v)}
        {}
        void print(std::ostream& ostream, ::mll::PrintContext context) final
        {
            PrintFunc(ostream, context, value);
        }
        ValueType const value;
    };

    explicit CustomType(ValueType value) : ::mll::Custom{std::make_shared<Data>(std::move(value))}
    {}

    CustomType(CustomType const& other) : ::mll::Custom{other}
    {}

    static std::optional<CustomType> from_node(::mll::Node const& node)
    {
        if (auto data = std::dynamic_pointer_cast<Data>(node.data())) {
            return CustomType{data};
        }
        return std::nullopt;
    }

    ValueType const& value() const
    {
        return std::static_pointer_cast<Data>(::mll::Custom::data())->value;
    }

private:
    CustomType(std::shared_ptr<Data> const& data) : ::mll::Custom{data}
    {}
};

} // namespace mll