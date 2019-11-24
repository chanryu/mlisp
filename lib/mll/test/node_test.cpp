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

TEST_CASE("Node can be casted", "[Node]")
{
    SECTION("Proc casting")
    {
        Proc proc{"x", [](List const&, Env&) -> Node { return nil; }};
        Node node = proc;
        REQUIRE(dynamic_node_cast<Proc>(node).has_value());

        REQUIRE(dynamic_node_cast<Proc>(node).has_value());
    }

    SECTION("Symbol casting")
    {
        Symbol symbol{"x"};
        Node node = symbol;
        REQUIRE(dynamic_node_cast<Symbol>(node).has_value());
    }

    SECTION("List casting")
    {
        List list{nil, nil};
        REQUIRE_FALSE(list.empty());

        Node node = list;
        REQUIRE(dynamic_node_cast<List>(node).has_value());
    }

    SECTION("nil can be casted to (empty) List")
    {
        Node node;
        REQUIRE(dynamic_node_cast<List>(node).has_value());
        REQUIRE(dynamic_node_cast<List>(node)->empty());
    }
}
} // namespace mll