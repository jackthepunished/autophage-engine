#pragma once

#include <autophage/ecs/system.hpp>
#include <autophage/window/window.hpp>

namespace autophage::ecs {

class RenderSystem : public System<RenderSystem>
{
public:
    explicit RenderSystem(IWindow& window);

    void update(World& world, f32 dt) override;

private:
    IWindow& window_;
};

}  // namespace autophage::ecs
