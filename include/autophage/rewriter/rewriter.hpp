#pragma once

#include <autophage/ecs/system.hpp>
#include <autophage/ecs/world.hpp>

#include <string>
#include <vector>

namespace autophage {
namespace ecs {
class World;
class ISystem;
}  // namespace ecs

namespace rewriter {

/// @brief Generates C++ code for ECS systems
class Rewriter
{
public:
    Rewriter() = default;

    /// @brief Generate a specialized update function for a query
    /// @param name The name of the generated function
    /// @param components The list of component type names
    /// @param logic The logic string to insert into the loop
    std::string generateSystemSource(const std::string& name,
                                     const std::vector<std::string>& components,
                                     const std::string& logic);

    /// @brief Generate a full System class implementation
    std::string generateSystemClass(const std::string& className, const std::string& logic);
};

/// @brief Proxy system that delegates to a JIT-compiled function
class JITSystem : public autophage::ecs::System<JITSystem>
{
public:
    using UpdateFunc = void (*)(autophage::ecs::World&, float);

    explicit JITSystem(UpdateFunc updateFunc)
        : autophage::ecs::System<JITSystem>("JITSystem"), updateFunc_(updateFunc)
    {
        (void)updateFunc;
    }

    void update(autophage::ecs::World& world, float dt) override
    {
        if (updateFunc_) {
            updateFunc_(world, dt);
        }
    }

private:
    UpdateFunc updateFunc_ = nullptr;
};

}  // namespace rewriter
}  // namespace autophage
