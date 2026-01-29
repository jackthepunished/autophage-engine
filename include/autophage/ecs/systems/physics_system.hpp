#pragma once

#include <autophage/ecs/components.hpp>
#include <autophage/ecs/system.hpp>


namespace autophage::ecs {

class PhysicsSystem : public System<PhysicsSystem>, public IVariantSystem
{
public:
    PhysicsSystem();

    void update(World& world, f32 dt) override;

    // IVariantSystem implementation
    [[nodiscard]] std::vector<SystemVariant> availableVariants() const override;
    [[nodiscard]] SystemVariant currentVariant() const noexcept override;
    bool switchVariant(SystemVariant variant) override;

private:
    void updateScalar(World& world, f32 dt);
    void updateSIMD(World& world, f32 dt);

    SystemVariant currentVariant_ = SystemVariant::Scalar;
};

}  // namespace autophage::ecs
