#include <autophage/core/logger.hpp>
#include <autophage/core/types.hpp>
#include <autophage/ecs/components.hpp>
#include <autophage/ecs/systems/render_system.hpp>
#include <autophage/ecs/world.hpp>
#include <autophage/window/window.hpp>

#include <chrono>
#include <thread>

using namespace autophage;

int main()
{
    // 1. Initialize Logger
    initLogger("AutophageDemo", LogLevel::Debug);
    LOG_INFO("Starting Autophage Engine Demo...");

    // 2. Create Window
    WindowConfig config;
    config.title = "Autophage Direct Demo";
    config.width = 800;
    config.height = 600;

    auto window = createWindow();
    if (!window || !window->init(config)) {
        LOG_ERROR("Failed to create window");
        return -1;
    }

    // 3. Setup ECS
    ecs::World world;

    // Register RenderSystem
    world.registerSystem<ecs::RenderSystem>(*window);

    // Initialize systems
    world.init();

    // 4. Create Entities

    // Entity 1: Red Box in center
    auto e1 = world.createEntity();
    world.addComponent<ecs::Transform>(
        e1, ecs::Transform(ecs::Vec3{400.0f, 300.0f, 0.0f}, ecs::Quat::identity(),
                           ecs::Vec3{50.0f, 50.0f, 1.0f}));
    world.addComponent<ecs::Renderable>(e1, ecs::Renderable(255, 0, 0, 255));  // Red
    world.addComponent<ecs::Visible>(e1);

    // Entity 2: Blue Box moving
    auto e2 = world.createEntity();
    world.addComponent<ecs::Transform>(
        e2, ecs::Transform(ecs::Vec3{100.0f, 100.0f, 0.0f}, ecs::Quat::identity(),
                           ecs::Vec3{30.0f, 30.0f, 1.0f}));
    world.addComponent<ecs::Renderable>(e2, ecs::Renderable(0, 0, 255, 255));  // Blue
    world.addComponent<ecs::Visible>(e2);
    world.addComponent<ecs::Velocity>(
        e2, ecs::Velocity(ecs::Vec3{150.0f, 150.0f, 0.0f}));  // Moving diagonal

    // 5. Game Loop
    LOG_INFO("Entering game loop...");

    auto lastTime = std::chrono::high_resolution_clock::now();
    bool running = true;

    while (running && !window->shouldClose()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        f32 dt = std::chrono::duration<f32>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Poll events
        window->pollEvents();

        // Simple update logic
        auto* t_ptr = world.getComponent<ecs::Transform>(e2);
        auto* v_ptr = world.getComponent<ecs::Velocity>(e2);

        if (t_ptr && v_ptr) {
            auto& pos = t_ptr->position;
            auto& vel = v_ptr->linear;

            pos.x += vel.x * dt;
            pos.y += vel.y * dt;

            // Bounce
            if (pos.x < 0 || pos.x > 800)
                vel.x *= -1.0f;
            if (pos.y < 0 || pos.y > 600)
                vel.y *= -1.0f;
        }

        // ECS Update (Render)
        world.update(dt);

        // Cap FPS ~60
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // Shutdown
    world.shutdown();
    shutdownLogger();

    return 0;
}
