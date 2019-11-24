#include <catch2/catch.hpp>

#include <mll/env.hpp>
#include <mll/list.hpp>
#include <mll/node.hpp>
#include <mll/proc.hpp>
#include <mll/symbol.hpp>

namespace mll {

TEST_CASE("Node is nil by default", "[Node]")
{
    Node node;

    REQUIRE(node.core() == nullptr);
}

} // namespace mll