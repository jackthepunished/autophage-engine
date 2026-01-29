#include <autophage/core/logger.hpp>
#include <autophage/ecs/systems/physics_system.hpp>
#include <autophage/optimizer/optimizer.hpp>


namespace autophage::optimizer {

Optimizer::Optimizer(analyzer::StatsCollector& stats) : stats_(stats) {}

void Optimizer::update(ecs::World& world)
{
    // 1. Analyze stats (stub)
    stats_.collect();
    auto hints = stats_.analyze();  // Returns generic hints

    // 2. Specific Rule: Physics System Optimization
    // Check entity count
    usize entityCount = world.entityCount();

    auto* physicsSystem = world.getSystem<ecs::PhysicsSystem>();
    if (physicsSystem) {
        auto current = physicsSystem->currentVariant();

        if (entityCount > 500 && current == ecs::SystemVariant::Scalar) {
            LOG_INFO(
                "[Optimizer] High entity count detected ({}). Switching PhysicsSystem to SIMD.",
                entityCount);
            physicsSystem->switchVariant(ecs::SystemVariant::SIMD);
        } else if (entityCount < 100 && current == ecs::SystemVariant::SIMD) {
            LOG_INFO(
                "[Optimizer] Low entity count detected ({}). Switching PhysicsSystem to Scalar.",
                entityCount);
            physicsSystem->switchVariant(ecs::SystemVariant::Scalar);
        }
    }
}

}  // namespace autophage::optimizer
