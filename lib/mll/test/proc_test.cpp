#include <catch2/catch.hpp>

#include <mll/proc.hpp>

namespace mll {

TEST_CASE("Proc can be casted from Node", "[Proc]")
{
    SECTION("Proc casting")
    {
        Proc proc{"x", {}};
        Node node = proc;
        REQUIRE(dynamic_node_cast<Proc>(node).has_value());
    }
}

} // namespace mll