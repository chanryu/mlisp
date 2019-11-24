#include <catch2/catch.hpp>

#include <mll/symbol.hpp>

TEST_CASE("Same name results same symbol", "[Symbol]")
{
    mll::Symbol sym1{"xyz"};
    mll::Symbol sym2{"xyz"};

    REQUIRE(sym1.core() == sym2.core());
}