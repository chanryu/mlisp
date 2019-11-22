#include <catch2/catch.hpp>

#include <mll/node.hpp>

TEST_CASE("node can be nil", "[Node]")
{
    mll::Node node;

    REQUIRE(node.core() == nullptr);
}