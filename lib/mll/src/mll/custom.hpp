#pragma once

#include <mll/node.hpp>

#include <ostream>

namespace mll {

class Parser;
enum class PrintContext;

class Custom {
public:
    Custom(Custom const&);

    struct Core : Node::Core {
        void accept(NodeVisitor&) final;
        virtual void print(std::ostream&, PrintContext) = 0;
    };
    std::shared_ptr<Core> const& core() const;

protected:
    explicit Custom(std::shared_ptr<Core> const&);

private:
    friend class Parser;
    std::shared_ptr<Core> _core;
};

template <typename ValueType, typename ValuePrinter>
class CustomType final : public Custom {
public:
    struct Core : Custom::Core {
        explicit Core(ValueType v) : value{std::move(v)}
        {}
        void print(std::ostream& ostream, PrintContext context) final
        {
            ValuePrinter::print(ostream, context, value);
        }
        ValueType const value;
    };

    explicit CustomType(ValueType value) : Custom{std::make_shared<Core>(std::move(value))}
    {}

    CustomType(CustomType const& other) : Custom{other}
    {}

    static std::optional<CustomType> from_node(Node const& node)
    {
        if (auto core = std::dynamic_pointer_cast<Core>(node.core())) {
            return CustomType{core};
        }
        return std::nullopt;
    }

    ValueType const& value() const
    {
        return std::static_pointer_cast<Core>(Custom::core())->value;
    }

private:
    CustomType(std::shared_ptr<Core> const& core) : Custom{core}
    {}
};

} // namespace mll