#include <catch2/catch.hpp>
#include <sstream>
#include "parser.hpp"

TEST_CASE("Parser throws exception upon unrecoverable error", "[Parser]")
{
    auto parse = [](char const* expr) {
        std::istringstream iss{expr};
        return mlisp::Parser{}.parse(iss);
    };

    SECTION("redundant closing parenthesis") {
        REQUIRE_THROWS_AS(parse(")"),  mll::ParseError);
        REQUIRE_THROWS_AS(parse("))"),  mll::ParseError);
    }

    SECTION("malformed string") {
        REQUIRE_THROWS_AS(parse(R"("abc)"),  mll::ParseError);
        REQUIRE_THROWS_AS(parse(R"("abc\")"),  mll::ParseError);
    }
}