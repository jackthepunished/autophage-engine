#pragma once

#include <autophage/ecs/world.hpp>
#include <autophage/rewriter/jit_compiler.hpp>

#include <memory>
#include <string>

namespace autophage::rewriter {

/// @brief Manages the hot-swapping of ECS systems
class HotSwapManager
{
public:
    explicit HotSwapManager(ecs::World& world);
    ~HotSwapManager();

    /// @brief Replace an existing system with a new one (statically compiled)
    /// @tparam T The system type to replace
    /// @tparam NewT The new system type
    template <typename T, typename NewT, typename... Args> void swapSystem(Args&&... args)
    {
        world_.replaceSystem<T, NewT>(std::forward<Args>(args)...);
    }

    /// @brief Hot-swap a system using JIT compiled code
    /// @param systemName The name of the system to replace
    /// @param source The C++ source code for the new implementation
    /// @return true if swap was successful
    bool hotSwapFromSource(const std::string& systemName, const std::string& source);

private:
    /// @brief A system implementation that delegates to JIT'd code
    class JITSystem : public ecs::System<JITSystem>
    {
    public:
        using UpdateFunc = void (*)(ecs::World&, f32);

        explicit JITSystem(UpdateFunc func) : System("JITSystem"), updateFunc_(func) {}

        void update(ecs::World& world, f32 dt) override
        {
            if (updateFunc_)
                updateFunc_(world, dt);
        }

    private:
        UpdateFunc updateFunc_;
    };

    ecs::World& world_;
    std::unique_ptr<JITCompiler> compiler_;
};

}  // namespace autophage::rewriter
