#include <catch2/catch.hpp>

#include <mll/list.hpp>

namespace mll {

TEST_CASE("List is nil by default", "[List]")
{
    List l;
    REQUIRE(l.empty());
}

TEST_CASE("List can be casted from Node", "[List]")
{
    SECTION("Node to List casting")
    {
        List list{nil, nil};
        REQUIRE_FALSE(list.empty());

        Node node = list;
        REQUIRE(dynamic_node_cast<List>(node).has_value());
    }

    SECTION("nil to List casting")
    {
        Node node;
        REQUIRE(dynamic_node_cast<List>(node).has_value());
        REQUIRE(dynamic_node_cast<List>(node)->empty());
    }
}
} // namespace mll