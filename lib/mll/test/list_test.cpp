#include <catch2/catch.hpp>

#include <mll/list.hpp>

TEST_CASE("List is nil by default", "[List]")
{
    mll::List l;

    REQUIRE(l.empty());
    REQUIRE(l == mll::nil);
}