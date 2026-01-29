/// @file test_component.cpp
/// @brief Tests for Component storage

#include <catch2/catch_test_macros.hpp>

#include <autophage/ecs/component_storage.hpp>

using namespace autophage;
using namespace autophage::ecs;

// Test component
struct Position {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Velocity {
    float vx = 0.0f;
    float vy = 0.0f;
    float vz = 0.0f;
};

struct Health {
    int value = 100;
};

TEST_CASE("ComponentArray basic operations", "[ecs][component]") {
    ComponentArray<Position> positions;
    Entity e1{0, 1};
    Entity e2{1, 1};

    SECTION("Initial state") {
        REQUIRE(positions.size() == 0);
        REQUIRE_FALSE(positions.has(e1));
    }

    SECTION("Set component") {
        Position& pos = positions.set(e1, {1.0f, 2.0f, 3.0f});
        
        REQUIRE(positions.size() == 1);
        REQUIRE(positions.has(e1));
        REQUIRE(pos.x == 1.0f);
        REQUIRE(pos.y == 2.0f);
        REQUIRE(pos.z == 3.0f);
    }

    SECTION("Get component") {
        positions.set(e1, {1.0f, 2.0f, 3.0f});
        
        Position* pos = positions.get(e1);
        REQUIRE(pos != nullptr);
        REQUIRE(pos->x == 1.0f);
        
        const Position* constPos = std::as_const(positions).get(e1);
        REQUIRE(constPos != nullptr);
        REQUIRE(constPos->x == 1.0f);
    }

    SECTION("Get non-existent returns nullptr") {
        REQUIRE(positions.get(e1) == nullptr);
    }

    SECTION("Replace existing component") {
        positions.set(e1, {1.0f, 2.0f, 3.0f});
        positions.set(e1, {4.0f, 5.0f, 6.0f});
        
        REQUIRE(positions.size() == 1);
        Position* pos = positions.get(e1);
        REQUIRE(pos->x == 4.0f);
    }

    SECTION("Multiple components") {
        positions.set(e1, {1.0f, 0.0f, 0.0f});
        positions.set(e2, {2.0f, 0.0f, 0.0f});
        
        REQUIRE(positions.size() == 2);
        REQUIRE(positions.get(e1)->x == 1.0f);
        REQUIRE(positions.get(e2)->x == 2.0f);
    }
}

TEST_CASE("ComponentArray removal", "[ecs][component]") {
    ComponentArray<Position> positions;
    Entity e1{0, 1};
    Entity e2{1, 1};
    Entity e3{2, 1};

    positions.set(e1, {1.0f, 0.0f, 0.0f});
    positions.set(e2, {2.0f, 0.0f, 0.0f});
    positions.set(e3, {3.0f, 0.0f, 0.0f});

    SECTION("Remove middle element") {
        positions.remove(e2);
        
        REQUIRE(positions.size() == 2);
        REQUIRE(positions.has(e1));
        REQUIRE_FALSE(positions.has(e2));
        REQUIRE(positions.has(e3));
        
        // Data integrity preserved
        REQUIRE(positions.get(e1)->x == 1.0f);
        REQUIRE(positions.get(e3)->x == 3.0f);
    }

    SECTION("Remove first element") {
        positions.remove(e1);
        
        REQUIRE(positions.size() == 2);
        REQUIRE(positions.get(e2)->x == 2.0f);
        REQUIRE(positions.get(e3)->x == 3.0f);
    }

    SECTION("Remove last element") {
        positions.remove(e3);
        
        REQUIRE(positions.size() == 2);
        REQUIRE(positions.get(e1)->x == 1.0f);
        REQUIRE(positions.get(e2)->x == 2.0f);
    }

    SECTION("Remove non-existent is safe") {
        Entity nonExistent{99, 1};
        positions.remove(nonExistent);
        REQUIRE(positions.size() == 3);
    }

    SECTION("Double remove is safe") {
        positions.remove(e1);
        positions.remove(e1);
        REQUIRE(positions.size() == 2);
    }
}

TEST_CASE("ComponentArray iteration", "[ecs][component]") {
    ComponentArray<Position> positions;
    Entity e1{0, 1};
    Entity e2{1, 1};
    Entity e3{2, 1};

    positions.set(e1, {1.0f, 0.0f, 0.0f});
    positions.set(e2, {2.0f, 0.0f, 0.0f});
    positions.set(e3, {3.0f, 0.0f, 0.0f});

    SECTION("forEach mutable") {
        positions.forEach([](Entity e, Position& pos) {
            pos.x *= 2.0f;
            (void)e;
        });

        REQUIRE(positions.get(e1)->x == 2.0f);
        REQUIRE(positions.get(e2)->x == 4.0f);
        REQUIRE(positions.get(e3)->x == 6.0f);
    }

    SECTION("forEach const") {
        float sum = 0.0f;
        std::as_const(positions).forEach([&sum](Entity e, const Position& pos) {
            sum += pos.x;
            (void)e;
        });

        REQUIRE(sum == 6.0f);
    }

    SECTION("entities() returns all entities") {
        auto entities = positions.entities();
        
        REQUIRE(entities.size() == 3);
        // Order may vary, so check presence
        REQUIRE(std::find(entities.begin(), entities.end(), e1) != entities.end());
        REQUIRE(std::find(entities.begin(), entities.end(), e2) != entities.end());
        REQUIRE(std::find(entities.begin(), entities.end(), e3) != entities.end());
    }
}

TEST_CASE("ComponentArray entity destruction callback", "[ecs][component]") {
    ComponentArray<Position> positions;
    Entity e1{0, 1};
    Entity e2{1, 1};

    positions.set(e1, {1.0f, 0.0f, 0.0f});
    positions.set(e2, {2.0f, 0.0f, 0.0f});

    positions.onEntityDestroyed(e1);

    REQUIRE(positions.size() == 1);
    REQUIRE_FALSE(positions.has(e1));
    REQUIRE(positions.has(e2));
}

TEST_CASE("ComponentRegistry", "[ecs][component]") {
    ComponentRegistry registry;

    SECTION("Register and get array") {
        registry.registerComponent<Position>();
        registry.registerComponent<Velocity>();

        REQUIRE(registry.isRegistered<Position>());
        REQUIRE(registry.isRegistered<Velocity>());
        REQUIRE_FALSE(registry.isRegistered<Health>());
    }

    SECTION("Get array auto-registers") {
        auto& positions = registry.getArray<Position>();
        REQUIRE(registry.isRegistered<Position>());

        Entity e{0, 1};
        positions.set(e, {1.0f, 2.0f, 3.0f});
        
        REQUIRE(registry.getArray<Position>().get(e)->x == 1.0f);
    }

    SECTION("Entity destruction notifies all arrays") {
        Entity e{0, 1};
        
        registry.getArray<Position>().set(e, {});
        registry.getArray<Velocity>().set(e, {});
        
        REQUIRE(registry.getArray<Position>().has(e));
        REQUIRE(registry.getArray<Velocity>().has(e));
        
        registry.onEntityDestroyed(e);
        
        REQUIRE_FALSE(registry.getArray<Position>().has(e));
        REQUIRE_FALSE(registry.getArray<Velocity>().has(e));
    }

    SECTION("Get array by TypeId") {
        registry.registerComponent<Position>();
        
        TypeId id = typeId<Position>();
        IComponentArray* array = registry.getArrayById(id);
        
        REQUIRE(array != nullptr);
        REQUIRE(array->componentType() == id);
    }

    SECTION("Clear removes all arrays") {
        registry.registerComponent<Position>();
        registry.registerComponent<Velocity>();
        registry.clear();
        
        REQUIRE_FALSE(registry.isRegistered<Position>());
        REQUIRE_FALSE(registry.isRegistered<Velocity>());
    }
}
