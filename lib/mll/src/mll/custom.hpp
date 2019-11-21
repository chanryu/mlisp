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

} // namespace mll

#define MLL_CUSTOM_TYPE_DECL(ClassName, ValueType)                                                                     \
    class ClassName final : public ::mll::Custom {                                                                     \
    public:                                                                                                            \
        explicit ClassName(ValueType);                                                                                 \
        ClassName(ClassName const&);                                                                                   \
        static std::optional<ClassName> from_node(::mll::Node const&);                                                 \
        ValueType const& value() const;                                                                                \
        struct Data : ::mll::Custom::Data {                                                                            \
            explicit Data(ValueType v);                                                                                \
            void print(std::ostream&, ::mll::PrintContext) final;                                                      \
            ValueType const value;                                                                                     \
        };                                                                                                             \
                                                                                                                       \
    private:                                                                                                           \
        ClassName(std::shared_ptr<Data> const&);                                                                       \
    }

#define MLL_CUSTOM_TYPE_IMPL(ClassName, ValueType, PrintFunc)                                                          \
    ClassName::ClassName(ValueType value) : ::mll::Custom{std::make_shared<Data>(std::move(value))}                    \
    {}                                                                                                                 \
    ClassName::ClassName(ClassName const& other) : ::mll::Custom{other}                                                \
    {}                                                                                                                 \
    ClassName::ClassName(std::shared_ptr<Data> const& data) : ::mll::Custom{data}                                      \
    {}                                                                                                                 \
    std::optional<ClassName> ClassName::from_node(::mll::Node const& node)                                             \
    {                                                                                                                  \
        if (auto data = std::dynamic_pointer_cast<Data>(node.data())) {                                                \
            return ClassName{data};                                                                                    \
        }                                                                                                              \
        return std::nullopt;                                                                                           \
    }                                                                                                                  \
    ValueType const& ClassName::value() const                                                                          \
    {                                                                                                                  \
        return std::static_pointer_cast<Data>(::mll::Custom::data())->value;                                           \
    }                                                                                                                  \
    ClassName::Data::Data(ValueType v) : value{std::move(v)}                                                           \
    {}                                                                                                                 \
    void ClassName::Data::print(std::ostream& ostream, ::mll::PrintContext context)                                    \
    {                                                                                                                  \
        PrintFunc(ostream, context, value);                                                                            \
    }