/// @file test_entity.cpp
/// @brief Tests for Entity and EntityManager

#include <autophage/ecs/entity.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace autophage;
using namespace autophage::ecs;

TEST_CASE("Entity creation and validity", "[ecs][entity]")
{
    SECTION("Default entity is invalid")
    {
        Entity e{};
        REQUIRE_FALSE(e.isValid());
    }

    SECTION("ecs::INVALID_ENTITY constant is invalid")
    {
        REQUIRE_FALSE(ecs::INVALID_ENTITY.isValid());
        REQUIRE(ecs::INVALID_ENTITY.index == 0);
        REQUIRE(ecs::INVALID_ENTITY.generation == 0);
    }

    SECTION("Entity with generation > 0 is valid")
    {
        Entity e{0, 1};
        REQUIRE(e.isValid());
    }
}

TEST_CASE("Entity comparison operators", "[ecs][entity]")
{
    Entity a{1, 1};
    Entity b{1, 1};
    Entity c{2, 1};
    Entity d{1, 2};

    SECTION("Equality")
    {
        REQUIRE(a == b);
        REQUIRE_FALSE(a == c);
        REQUIRE_FALSE(a == d);
    }

    SECTION("Inequality")
    {
        REQUIRE_FALSE(a != b);
        REQUIRE(a != c);
        REQUIRE(a != d);
    }

    SECTION("Less than ordering")
    {
        REQUIRE(a < c);  // Different index
        REQUIRE(a < d);  // Same index, different generation
        REQUIRE_FALSE(c < a);
    }
}

TEST_CASE("EntityManager creation", "[ecs][entity]")
{
    EntityManager manager;

    SECTION("Initial state")
    {
        REQUIRE(manager.aliveCount() == 0);
        REQUIRE(manager.capacity() == 0);
        REQUIRE(manager.recycledCount() == 0);
    }

    SECTION("Create single entity")
    {
        Entity e = manager.create();
        REQUIRE(e.isValid());
        REQUIRE(e.index == 0);
        REQUIRE(e.generation == 1);
        REQUIRE(manager.aliveCount() == 1);
        REQUIRE(manager.isAlive(e));
    }

    SECTION("Create multiple entities")
    {
        Entity e1 = manager.create();
        Entity e2 = manager.create();
        Entity e3 = manager.create();

        REQUIRE(manager.aliveCount() == 3);
        REQUIRE(e1.index == 0);
        REQUIRE(e2.index == 1);
        REQUIRE(e3.index == 2);

        REQUIRE(manager.isAlive(e1));
        REQUIRE(manager.isAlive(e2));
        REQUIRE(manager.isAlive(e3));
    }
}

TEST_CASE("EntityManager destruction", "[ecs][entity]")
{
    EntityManager manager;

    SECTION("Destroy single entity")
    {
        Entity e = manager.create();
        REQUIRE(manager.isAlive(e));

        bool destroyed = manager.destroy(e);
        REQUIRE(destroyed);
        REQUIRE_FALSE(manager.isAlive(e));
        REQUIRE(manager.aliveCount() == 0);
        REQUIRE(manager.recycledCount() == 1);
    }

    SECTION("Double destroy returns false")
    {
        Entity e = manager.create();
        REQUIRE(manager.destroy(e));
        REQUIRE_FALSE(manager.destroy(e));
    }

    SECTION("Destroy invalid entity returns false")
    {
        REQUIRE_FALSE(manager.destroy(ecs::INVALID_ENTITY));
    }
}

TEST_CASE("EntityManager recycling", "[ecs][entity]")
{
    EntityManager manager;

    SECTION("Recycled entity has incremented generation")
    {
        Entity e1 = manager.create();
        REQUIRE(e1.generation == 1);

        manager.destroy(e1);
        Entity e2 = manager.create();

        // Same index, different generation
        REQUIRE(e2.index == e1.index);
        REQUIRE(e2.generation == 2);

        // Old entity should not be alive
        REQUIRE_FALSE(manager.isAlive(e1));
        REQUIRE(manager.isAlive(e2));
    }

    SECTION("LIFO recycling order")
    {
        Entity e1 = manager.create();
        Entity e2 = manager.create();
        Entity e3 = manager.create();

        manager.destroy(e1);
        manager.destroy(e2);
        manager.destroy(e3);

        // Should recycle in reverse order (LIFO)
        Entity r1 = manager.create();
        Entity r2 = manager.create();
        Entity r3 = manager.create();

        REQUIRE(r1.index == e3.index);
        REQUIRE(r2.index == e2.index);
        REQUIRE(r3.index == e1.index);
    }
}

TEST_CASE("EntityManager forEach", "[ecs][entity]")
{
    EntityManager manager;

    Entity e1 = manager.create();
    Entity e2 = manager.create();
    Entity e3 = manager.create();
    manager.destroy(e2);

    SECTION("Iterates only alive entities")
    {
        std::vector<Entity> visited;
        manager.forEach([&](Entity e) { visited.push_back(e); });

        REQUIRE(visited.size() == 2);
        REQUIRE(std::find(visited.begin(), visited.end(), e1) != visited.end());
        REQUIRE(std::find(visited.begin(), visited.end(), e3) != visited.end());
        REQUIRE(std::find(visited.begin(), visited.end(), e2) == visited.end());
    }
}

TEST_CASE("EntityManager reserve and clear", "[ecs][entity]")
{
    EntityManager manager;

    SECTION("Reserve capacity")
    {
        manager.reserve(100);
        // Reserve doesn't create entities
        REQUIRE(manager.aliveCount() == 0);
    }

    SECTION("Clear resets manager")
    {
        (void)manager.create();
        (void)manager.create();
        (void)manager.create();

        manager.clear();

        REQUIRE(manager.aliveCount() == 0);
        REQUIRE(manager.capacity() == 0);
        REQUIRE(manager.recycledCount() == 0);
    }
}

TEST_CASE("Entity hashing", "[ecs][entity]")
{
    std::hash<Entity> hasher;

    Entity e1{1, 1};
    Entity e2{1, 1};
    Entity e3{2, 1};

    SECTION("Same entities have same hash")
    {
        REQUIRE(hasher(e1) == hasher(e2));
    }

    SECTION("Different entities likely have different hash")
    {
        // Not guaranteed, but highly likely
        REQUIRE(hasher(e1) != hasher(e3));
    }
}
