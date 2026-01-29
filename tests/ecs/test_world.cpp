/// @file test_world.cpp
/// @brief Tests for ECS World

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <autophage/ecs/world.hpp>
#include <autophage/ecs/components.hpp>

using namespace autophage;
using namespace autophage::ecs;

// Simple test components
struct TestPosition {
    float x = 0.0f;
    float y = 0.0f;
};

struct TestVelocity {
    float vx = 0.0f;
    float vy = 0.0f;
};

struct TestHealth {
    int hp = 100;
};

TEST_CASE("World entity management", "[ecs][world]") {
    World world;

    SECTION("Create entities") {
        Entity e1 = world.createEntity();
        Entity e2 = world.createEntity();

        REQUIRE(e1.isValid());
        REQUIRE(e2.isValid());
        REQUIRE(e1 != e2);
        REQUIRE(world.entityCount() == 2);
    }

    SECTION("Destroy entities") {
        Entity e = world.createEntity();
        REQUIRE(world.isAlive(e));

        world.destroyEntity(e);
        REQUIRE_FALSE(world.isAlive(e));
        REQUIRE(world.entityCount() == 0);
    }

    SECTION("Destroying entity removes components") {
        Entity e = world.createEntity();
        world.addComponent<TestPosition>(e, {1.0f, 2.0f});
        world.addComponent<TestHealth>(e, {50});

        REQUIRE(world.hasComponent<TestPosition>(e));
        REQUIRE(world.hasComponent<TestHealth>(e));

        world.destroyEntity(e);

        // After destruction, has() should return false
        // (entity is invalid, so component lookup fails)
        REQUIRE_FALSE(world.hasComponent<TestPosition>(e));
        REQUIRE_FALSE(world.hasComponent<TestHealth>(e));
    }
}

TEST_CASE("World component management", "[ecs][world]") {
    World world;
    Entity e = world.createEntity();

    SECTION("Add and get component") {
        TestPosition& pos = world.addComponent<TestPosition>(e, {10.0f, 20.0f});

        REQUIRE(pos.x == 10.0f);
        REQUIRE(pos.y == 20.0f);

        TestPosition* retrieved = world.getComponent<TestPosition>(e);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->x == 10.0f);
    }

    SECTION("Has component") {
        REQUIRE_FALSE(world.hasComponent<TestPosition>(e));

        world.addComponent<TestPosition>(e);
        REQUIRE(world.hasComponent<TestPosition>(e));
    }

    SECTION("Remove component") {
        world.addComponent<TestPosition>(e);
        REQUIRE(world.hasComponent<TestPosition>(e));

        world.removeComponent<TestPosition>(e);
        REQUIRE_FALSE(world.hasComponent<TestPosition>(e));
    }

    SECTION("Multiple components on entity") {
        world.addComponent<TestPosition>(e, {1.0f, 2.0f});
        world.addComponent<TestVelocity>(e, {3.0f, 4.0f});
        world.addComponent<TestHealth>(e, {75});

        REQUIRE(world.hasComponent<TestPosition>(e));
        REQUIRE(world.hasComponent<TestVelocity>(e));
        REQUIRE(world.hasComponent<TestHealth>(e));

        REQUIRE(world.getComponent<TestPosition>(e)->x == 1.0f);
        REQUIRE(world.getComponent<TestVelocity>(e)->vx == 3.0f);
        REQUIRE(world.getComponent<TestHealth>(e)->hp == 75);
    }

    SECTION("Get non-existent component returns nullptr") {
        REQUIRE(world.getComponent<TestPosition>(e) == nullptr);
    }

    SECTION("Const getComponent") {
        world.addComponent<TestPosition>(e, {5.0f, 6.0f});

        const World& constWorld = world;
        const TestPosition* pos = constWorld.getComponent<TestPosition>(e);

        REQUIRE(pos != nullptr);
        REQUIRE(pos->x == 5.0f);
    }
}

TEST_CASE("World query", "[ecs][world]") {
    World world;

    // Create some entities with various components
    Entity e1 = world.createEntity();
    world.addComponent<TestPosition>(e1, {1.0f, 0.0f});
    world.addComponent<TestVelocity>(e1, {0.1f, 0.0f});

    Entity e2 = world.createEntity();
    world.addComponent<TestPosition>(e2, {2.0f, 0.0f});
    world.addComponent<TestVelocity>(e2, {0.2f, 0.0f});
    world.addComponent<TestHealth>(e2, {100});

    Entity e3 = world.createEntity();
    world.addComponent<TestPosition>(e3, {3.0f, 0.0f});
    // No velocity

    SECTION("Query single component") {
        auto q = world.query<TestPosition>();
        int count = 0;
        q.forEach([&count](Entity e, TestPosition& pos) {
            count++;
            (void)e;
            (void)pos;
        });
        REQUIRE(count == 3);
    }

    SECTION("Query multiple components") {
        auto q = world.query<TestPosition, TestVelocity>();
        int count = 0;
        q.forEach([&count](Entity e, TestPosition& pos, TestVelocity& vel) {
            count++;
            (void)e;
            (void)pos;
            (void)vel;
        });
        REQUIRE(count == 2);  // Only e1 and e2 have both
    }

    SECTION("Query with modification") {
        auto q = world.query<TestPosition, TestVelocity>();
        q.forEach([](Entity e, TestPosition& pos, TestVelocity& vel) {
            pos.x += vel.vx;
            (void)e;
        });

        REQUIRE(world.getComponent<TestPosition>(e1)->x == Catch::Approx(1.1f));
        REQUIRE(world.getComponent<TestPosition>(e2)->x == Catch::Approx(2.2f));
    }

    SECTION("Query entities()") {
        auto q = world.query<TestPosition, TestVelocity>();
        auto entities = q.entities();

        REQUIRE(entities.size() == 2);
        REQUIRE(std::find(entities.begin(), entities.end(), e1) != entities.end());
        REQUIRE(std::find(entities.begin(), entities.end(), e2) != entities.end());
    }

    SECTION("Query count()") {
        auto q = world.query<TestPosition, TestVelocity>();
        REQUIRE(q.count() == 2);
    }

    SECTION("Query any()") {
        auto q1 = world.query<TestPosition, TestVelocity>();
        REQUIRE(q1.any());

        auto q2 = world.query<TestHealth>();
        REQUIRE(q2.any());

        // No entity has all three
        // Actually e2 does have all three - let's use a component no one has
    }
}

TEST_CASE("EntityBuilder fluent API", "[ecs][world]") {
    World world;

    SECTION("Build entity with components") {
        Entity e = createEntity(world)
            .with<TestPosition>({10.0f, 20.0f})
            .with<TestVelocity>({1.0f, 2.0f})
            .with<TestHealth>({80})
            .build();

        REQUIRE(e.isValid());
        REQUIRE(world.hasComponent<TestPosition>(e));
        REQUIRE(world.hasComponent<TestVelocity>(e));
        REQUIRE(world.hasComponent<TestHealth>(e));

        REQUIRE(world.getComponent<TestPosition>(e)->x == 10.0f);
        REQUIRE(world.getComponent<TestHealth>(e)->hp == 80);
    }

    SECTION("Implicit conversion to Entity") {
        Entity e = createEntity(world)
            .with<TestPosition>({5.0f, 5.0f});

        REQUIRE(e.isValid());
        REQUIRE(world.hasComponent<TestPosition>(e));
    }
}

TEST_CASE("World lifecycle", "[ecs][world]") {
    World world;

    SECTION("Clear removes all entities and components") {
        Entity e1 = world.createEntity();
        Entity e2 = world.createEntity();
        world.addComponent<TestPosition>(e1);
        world.addComponent<TestPosition>(e2);

        world.clear();

        REQUIRE(world.entityCount() == 0);
        REQUIRE_FALSE(world.isAlive(e1));
        REQUIRE_FALSE(world.isAlive(e2));
    }

    SECTION("Reserve entities") {
        world.reserveEntities(1000);
        // Just verify no crash
        Entity e = world.createEntity();
        REQUIRE(e.isValid());
    }
}

TEST_CASE("World with Transform and Velocity components", "[ecs][world]") {
    World world;

    Entity e = world.createEntity();
    world.addComponent<Transform>(e, Transform{Vec3{1.0f, 2.0f, 3.0f}});
    world.addComponent<Velocity>(e, Velocity{Vec3{0.1f, 0.2f, 0.3f}});

    SECTION("Components stored correctly") {
        Transform* t = world.getComponent<Transform>(e);
        Velocity* v = world.getComponent<Velocity>(e);

        REQUIRE(t != nullptr);
        REQUIRE(v != nullptr);

        REQUIRE(t->position.x == Catch::Approx(1.0f));
        REQUIRE(t->position.y == Catch::Approx(2.0f));
        REQUIRE(t->position.z == Catch::Approx(3.0f));

        REQUIRE(v->linear.x == Catch::Approx(0.1f));
        REQUIRE(v->linear.y == Catch::Approx(0.2f));
        REQUIRE(v->linear.z == Catch::Approx(0.3f));
    }

    SECTION("Manual velocity integration") {
        float dt = 1.0f;
        
        auto q = world.query<Transform, Velocity>();
        q.forEach([dt](Entity /*e*/, Transform& t, Velocity& v) {
            t.position.x += v.linear.x * dt;
            t.position.y += v.linear.y * dt;
            t.position.z += v.linear.z * dt;
        });

        Transform* t = world.getComponent<Transform>(e);
        REQUIRE(t->position.x == Catch::Approx(1.1f));
        REQUIRE(t->position.y == Catch::Approx(2.2f));
        REQUIRE(t->position.z == Catch::Approx(3.3f));
    }
}
