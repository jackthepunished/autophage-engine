#include <autophage/ecs/components.hpp>
#include <autophage/ecs/systems/render_system.hpp>
#include <autophage/ecs/world.hpp>

namespace autophage::ecs {

RenderSystem::RenderSystem(IWindow& window) : System("RenderSystem"), window_(window) {}

void RenderSystem::update(World& world, [[maybe_unused]] f32 dt)
{
    // Clear screen
    window_.clear(0, 0, 0, 255);  // Black background

    // Iterate all entities with Transform and Renderable
    // Use the type-safe view API
    for (auto [entity, transform, renderable] : world.view<Transform, Renderable>()) {
        if (world.hasComponent<Visible>(entity)) {
            // Project valid entities
            // For now, map World X/Y directly to Screen X/Y
            // Treat 0,0 as center? Or top-left?
            // WindowSDL 0,0 is top-left.
            // Let's assume Transform is in pixels for this phase.

            i32 x = static_cast<i32>(transform.position.x);
            i32 y = static_cast<i32>(transform.position.y);
            i32 w = static_cast<i32>(transform.scale.x);
            i32 h = static_cast<i32>(transform.scale.y);

            window_.drawRect(x, y, w, h, renderable.r, renderable.g, renderable.b, renderable.a);
        }
    }

    // Present frame
    window_.present();
}

}  // namespace autophage::ecs
