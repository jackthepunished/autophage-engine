/// @file test_types.cpp
/// @brief Tests for core type definitions

#include <autophage/core/types.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace autophage;

TEST_CASE("Integer type sizes", "[core][types]")
{
    REQUIRE(sizeof(i8) == 1);
    REQUIRE(sizeof(i16) == 2);
    REQUIRE(sizeof(i32) == 4);
    REQUIRE(sizeof(i64) == 8);

    REQUIRE(sizeof(u8) == 1);
    REQUIRE(sizeof(u16) == 2);
    REQUIRE(sizeof(u32) == 4);
    REQUIRE(sizeof(u64) == 8);
}

TEST_CASE("Entity", "[core][types]")
{
    SECTION("Default construction creates invalid entity")
    {
        Entity entity{};
        REQUIRE_FALSE(entity.isValid());
        REQUIRE(entity == INVALID_ENTITY);
    }

    SECTION("Valid entity with non-zero generation")
    {
        Entity entity{42, 1};
        REQUIRE(entity.isValid());
        REQUIRE(entity.index == 42);
        REQUIRE(entity.generation == 1);
    }

    SECTION("Comparison operators")
    {
        Entity a{1, 1};
        Entity b{1, 1};
        Entity c{2, 1};
        Entity d{1, 2};

        REQUIRE(a == b);
        REQUIRE(a != c);
        REQUIRE(a != d);
        REQUIRE(a < c);
        REQUIRE(a < d);
    }
}

TEST_CASE("ErrorCode toString", "[core][types]")
{
    REQUIRE(toString(ErrorCode::Success) == "Success");
    REQUIRE(toString(ErrorCode::NotFound) == "NotFound");
    REQUIRE(toString(ErrorCode::OutOfMemory) == "OutOfMemory");
}

#if !AUTOPHAGE_HAS_EXPECTED
TEST_CASE("Result fallback type", "[core][types]")
{
    SECTION("Successful result")
    {
        Result<int> result{42};
        REQUIRE(result.hasValue());
        REQUIRE(result);
        REQUIRE(result.value() == 42);
    }

    SECTION("Error result")
    {
        Result<int> result{ErrorCode::NotFound};
        REQUIRE_FALSE(result.hasValue());
        REQUIRE_FALSE(result);
        REQUIRE(result.error() == ErrorCode::NotFound);
    }

    SECTION("valueOr returns default on error")
    {
        Result<int> result{ErrorCode::NotFound};
        REQUIRE(result.valueOr(100) == 100);
    }
}

TEST_CASE("VoidResult fallback type", "[core][types]")
{
    SECTION("Success")
    {
        VoidResult result{};
        REQUIRE(result.hasValue());
        REQUIRE(result);
    }

    SECTION("Error")
    {
        VoidResult result{ErrorCode::InvalidState};
        REQUIRE_FALSE(result.hasValue());
        REQUIRE(result.error() == ErrorCode::InvalidState);
    }
}
#endif
