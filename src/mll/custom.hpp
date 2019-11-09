#pragma once

#include <mll/node.hpp>
#include <ostream>

namespace mll {

class Parser;

class Custom {
public:
    struct Data : Node::Data {
        void accept(NodeVisitor&) override;
        virtual void print(std::ostream&) = 0;
    };
    std::shared_ptr<Data> const& data() const;

protected:
    explicit Custom(std::shared_ptr<Data>);

private:
    std::shared_ptr<Data> data_;

    friend class Node;
    friend class Parser;
};

} // namespace mll