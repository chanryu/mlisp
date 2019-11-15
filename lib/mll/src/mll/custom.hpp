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
        void accept(NodeVisitor&) override;
        virtual void print(std::ostream&, PrintContext) = 0;
    };
    std::shared_ptr<Data> const& data() const;

protected:
    explicit Custom(std::shared_ptr<Data> const&);

private:
    friend class Parser;
    std::shared_ptr<Data> data_;
};

} // namespace mll