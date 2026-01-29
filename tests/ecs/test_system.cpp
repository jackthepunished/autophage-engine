/// @file test_system.cpp
/// @brief Tests for ECS Systems

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <autophage/ecs/world.hpp>
#include <autophage/ecs/system.hpp>
#include <autophage/ecs/components.hpp>
#include <autophage/ecs/systems.hpp>

using namespace autophage;
using namespace autophage::ecs;

// Simple test system
class CounterSystem : public System<CounterSystem> {
public:
    CounterSystem() : System("CounterSystem") {}

    void update(World& world, f32 dt) override {
        updateCount++;
        lastDt = dt;
        lastEntityCount = world.entityCount();
    }

    int updateCount = 0;
    f32 lastDt = 0.0f;
    usize lastEntityCount = 0;
};

// System that modifies components
class PositionModifierSystem : public System<PositionModifierSystem> {
public:
    PositionModifierSystem() : System("PositionModifierSystem") {}

    void update(World& world, f32 dt) override {
        auto q = world.query<Transform>();
        q.forEach([dt](Entity /*e*/, Transform& t) {
            t.position.x += 1.0f * dt;
        });
    }
};

TEST_CASE("System base class", "[ecs][system]") {
    CounterSystem system;

    SECTION("System properties") {
        REQUIRE(system.name() == std::string("CounterSystem"));
        REQUIRE(system.systemId() == typeId<CounterSystem>());
    }

    SECTION("Enable/disable") {
        REQUIRE(system.isEnabled());

        system.setEnabled(false);
        REQUIRE_FALSE(system.isEnabled());

        system.setEnabled(true);
        REQUIRE(system.isEnabled());
    }
}

TEST_CASE("SystemRegistry", "[ecs][system]") {
    SystemRegistry registry;
    World world;

    SECTION("Register and get system") {
        CounterSystem& counter = registry.registerSystem<CounterSystem>();

        REQUIRE(registry.count() == 1);

        CounterSystem* retrieved = registry.getSystem<CounterSystem>();
        REQUIRE(retrieved == &counter);
    }

    SECTION("Get non-existent system returns nullptr") {
        REQUIRE(registry.getSystem<CounterSystem>() == nullptr);
    }

    SECTION("Multiple systems") {
        registry.registerSystem<CounterSystem>();
        registry.registerSystem<PositionModifierSystem>();

        REQUIRE(registry.count() == 2);
        REQUIRE(registry.getSystem<CounterSystem>() != nullptr);
        REQUIRE(registry.getSystem<PositionModifierSystem>() != nullptr);
    }

    SECTION("initAll") {
        registry.registerSystem<CounterSystem>();
        registry.initAll(world);
        // CounterSystem has empty init, just verify no crash
    }

    SECTION("updateAll") {
        CounterSystem& counter = registry.registerSystem<CounterSystem>();

        registry.updateAll(world, 0.016f);
        REQUIRE(counter.updateCount == 1);
        REQUIRE(counter.lastDt == Catch::Approx(0.016f));

        registry.updateAll(world, 0.032f);
        REQUIRE(counter.updateCount == 2);
        REQUIRE(counter.lastDt == Catch::Approx(0.032f));
    }

    SECTION("updateAll respects enabled flag") {
        CounterSystem& counter = registry.registerSystem<CounterSystem>();
        counter.setEnabled(false);

        registry.updateAll(world, 0.016f);
        REQUIRE(counter.updateCount == 0);
    }

    SECTION("shutdownAll") {
        registry.registerSystem<CounterSystem>();
        registry.shutdownAll(world);
        // Just verify no crash
    }

    SECTION("clear") {
        registry.registerSystem<CounterSystem>();
        registry.clear();
        REQUIRE(registry.count() == 0);
    }
}

TEST_CASE("World with systems", "[ecs][system]") {
    World world;

    SECTION("Register system via world") {
        PositionModifierSystem& system = world.registerSystem<PositionModifierSystem>();

        REQUIRE(world.getSystem<PositionModifierSystem>() == &system);
    }

    SECTION("System modifies entities") {
        world.registerSystem<PositionModifierSystem>();

        Entity e = world.createEntity();
        world.addComponent<Transform>(e, Transform{Vec3{0.0f, 0.0f, 0.0f}});

        world.updateSystems(1.0f);

        Transform* t = world.getComponent<Transform>(e);
        REQUIRE(t->position.x == Catch::Approx(1.0f));

        world.updateSystems(0.5f);
        REQUIRE(t->position.x == Catch::Approx(1.5f));
    }

    SECTION("World init/update/shutdown lifecycle") {
        CounterSystem& counter = world.registerSystem<CounterSystem>();

        world.init();
        REQUIRE(counter.updateCount == 0);

        world.update(0.016f);
        REQUIRE(counter.updateCount == 1);

        world.shutdown();
        // Just verify completes without crash
    }
}

TEST_CASE("SystemVariant enum", "[ecs][system]") {
    SECTION("toString") {
        REQUIRE(toString(SystemVariant::Scalar) == std::string("Scalar"));
        REQUIRE(toString(SystemVariant::SIMD) == std::string("SIMD"));
        REQUIRE(toString(SystemVariant::GPU) == std::string("GPU"));
        REQUIRE(toString(SystemVariant::Approximate) == std::string("Approximate"));
    }
}

TEST_CASE("VelocitySystem", "[ecs][system]") {
    World world;

    SECTION("Scalar variant") {
        VelocitySystemScalar& system = world.registerSystem<VelocitySystemScalar>();

        Entity e = world.createEntity();
        world.addComponent<Transform>(e, Transform{Vec3{0.0f, 0.0f, 0.0f}});
        world.addComponent<Velocity>(e, Velocity{Vec3{10.0f, 20.0f, 30.0f}});

        system.update(world, 0.1f);

        Transform* t = world.getComponent<Transform>(e);
        REQUIRE(t->position.x == Catch::Approx(1.0f));
        REQUIRE(t->position.y == Catch::Approx(2.0f));
        REQUIRE(t->position.z == Catch::Approx(3.0f));
    }

    SECTION("Variant system hot-swap") {
        VelocitySystem& system = world.registerSystem<VelocitySystem>();

        // Check available variants
        auto variants = system.availableVariants();
        REQUIRE(variants.size() >= 1);  // At least Scalar

        // Scalar should always be available
        REQUIRE(system.switchVariant(SystemVariant::Scalar));
        REQUIRE(system.currentVariant() == SystemVariant::Scalar);

        Entity e = world.createEntity();
        world.addComponent<Transform>(e, Transform{Vec3{0.0f, 0.0f, 0.0f}});
        world.addComponent<Velocity>(e, Velocity{Vec3{10.0f, 0.0f, 0.0f}});

        system.update(world, 1.0f);

        Transform* t = world.getComponent<Transform>(e);
        REQUIRE(t->position.x == Catch::Approx(10.0f));
    }

    #if defined(AUTOPHAGE_SIMD_AVX2) || defined(AUTOPHAGE_SIMD_SSE2)
    SECTION("SIMD variant") {
        VelocitySystem& system = world.registerSystem<VelocitySystem>();

        REQUIRE(system.switchVariant(SystemVariant::SIMD));
        REQUIRE(system.currentVariant() == SystemVariant::SIMD);

        Entity e = world.createEntity();
        world.addComponent<Transform>(e, Transform{Vec3{0.0f, 0.0f, 0.0f}});
        world.addComponent<Velocity>(e, Velocity{Vec3{10.0f, 20.0f, 30.0f}});

        system.update(world, 0.1f);

        Transform* t = world.getComponent<Transform>(e);
        REQUIRE(t->position.x == Catch::Approx(1.0f));
        REQUIRE(t->position.y == Catch::Approx(2.0f));
        REQUIRE(t->position.z == Catch::Approx(3.0f));
    }
    #endif
}

TEST_CASE("GravitySystem", "[ecs][system]") {
    World world;
    GravitySystem& system = world.registerSystem<GravitySystem>();

    Entity e = world.createEntity();
    world.addComponent<Velocity>(e, Velocity{Vec3{0.0f, 0.0f, 0.0f}});
    world.addComponent<Mass>(e, Mass{1.0f});

    SECTION("Default gravity") {
        system.update(world, 1.0f);

        Velocity* v = world.getComponent<Velocity>(e);
        REQUIRE(v->linear.y == Catch::Approx(-9.81f));
    }

    SECTION("Custom gravity") {
        world.addComponent<Gravity>(e, Gravity{Vec3{0.0f, -5.0f, 0.0f}});

        system.update(world, 1.0f);

        Velocity* v = world.getComponent<Velocity>(e);
        REQUIRE(v->linear.y == Catch::Approx(-5.0f));
    }
}

TEST_CASE("CleanupSystem", "[ecs][system]") {
    World world;
    CleanupSystem& system = world.registerSystem<CleanupSystem>();

    Entity e1 = world.createEntity();
    Entity e2 = world.createEntity();
    Entity e3 = world.createEntity();

    world.addComponent<Transform>(e1);
    world.addComponent<Transform>(e2);
    world.addComponent<Transform>(e3);

    // Mark e2 for destruction
    world.addComponent<Destroyed>(e2);

    REQUIRE(world.entityCount() == 3);

    system.update(world, 0.0f);

    REQUIRE(world.entityCount() == 2);
    REQUIRE(world.isAlive(e1));
    REQUIRE_FALSE(world.isAlive(e2));
    REQUIRE(world.isAlive(e3));
}
