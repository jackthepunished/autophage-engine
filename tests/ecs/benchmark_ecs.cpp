#define ANKERL_NANOBENCH_IMPLEMENTATION
#include <autophage/ecs/components.hpp>
#include <autophage/ecs/systems.hpp>
#include <autophage/ecs/world.hpp>

#include <nanobench.h>


using namespace autophage;
using namespace autophage::ecs;

// Benchmark System
class BenchmarkSystem : public System<BenchmarkSystem>
{
public:
    BenchmarkSystem() : System("BenchmarkSystem") {}

    void update(World& world, f32 dt) override
    {
        auto q = world.query<Transform, Velocity>();
        q.forEach(
            [dt](Entity /*e*/, Transform& t, Velocity& v) { t.position.x += v.linear.x * dt; });
    }
};

int main()
{
    World world;
    world.registerSystem<BenchmarkSystem>();

    // Setup
    const int entityCount = 100000;
    world.reserveEntities(entityCount);

    // Register components to avoid allocation during loop
    world.registerComponent<Transform>();
    world.registerComponent<Velocity>();
    world.componentRegistry().getArray<Transform>().reserve(entityCount);
    world.componentRegistry().getArray<Velocity>().reserve(entityCount);

    for (int i = 0; i < entityCount; ++i) {
        Entity e = world.createEntity();
        world.addComponent<Transform>(e, Transform{Vec3{0, 0, 0}});
        world.addComponent<Velocity>(e, Velocity{Vec3{1, 1, 1}});
    }

    ankerl::nanobench::Bench().minEpochIterations(100).run("ECS Iteration (100k entities)",
                                                           [&] { world.update(0.016f); });

    return 0;
}
