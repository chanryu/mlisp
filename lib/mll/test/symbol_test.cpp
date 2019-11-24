#include <catch2/catch.hpp>

#include <mll/symbol.hpp>

namespace mll {

TEST_CASE("Same name results same symbol", "[Symbol]")
{
    Symbol sym1{"xyz"};
    Symbol sym2{"xyz"};

    REQUIRE(sym1.core() == sym2.core());
}

TEST_CASE("Symbol can be casted from Node", "[Symbol]")
{
    SECTION("Symbol casting")
    {
        Symbol symbol{"x"};
        Node node = symbol;
        REQUIRE(dynamic_node_cast<Symbol>(node).has_value());
    }
}

} // namespace mll