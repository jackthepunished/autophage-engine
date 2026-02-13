#pragma once

/// @file system.hpp
/// @brief System definitions for ECS

#include <autophage/core/type_id.hpp>
#include <autophage/core/types.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace autophage::ecs {

// Forward declarations
class World;

// =============================================================================
// System Interface
// =============================================================================

/// @brief Base interface for all systems
class ISystem
{
public:
    virtual ~ISystem() = default;

    /// @brief Get the system's unique identifier
    [[nodiscard]] virtual TypeId systemId() const noexcept = 0;

    /// @brief Get the system's name (for debugging)
    [[nodiscard]] virtual const char* name() const noexcept = 0;

    /// @brief Initialize the system
    virtual void init(World& world) = 0;

    /// @brief Update the system
    /// @param world The ECS world
    /// @param dt Delta time in seconds
    virtual void update(World& world, f32 dt) = 0;

    /// @brief Shutdown the system
    virtual void shutdown(World& world) = 0;

    /// @brief Check if the system is enabled
    [[nodiscard]] virtual bool isEnabled() const noexcept = 0;

    /// @brief Enable or disable the system
    virtual void setEnabled(bool enabled) = 0;
};

// =============================================================================
// System Base Class
// =============================================================================

/// @brief Base class for systems with common functionality
/// @tparam Derived CRTP derived class
/// @brief Base class for systems with common functionality
/// @tparam Derived CRTP derived class
template <typename Derived> class System : public virtual ISystem
{
public:
    [[nodiscard]] TypeId systemId() const noexcept override { return typeId<Derived>(); }

    [[nodiscard]] const char* name() const noexcept override { return name_.c_str(); }

    void init([[maybe_unused]] World& world) override {}
    void shutdown([[maybe_unused]] World& world) override {}

    [[nodiscard]] bool isEnabled() const noexcept override { return enabled_; }

    void setEnabled(bool enabled) override { enabled_ = enabled; }

protected:
    explicit System(String name = "UnnamedSystem") : name_(std::move(name)) {}

    String name_;
    bool enabled_ = true;
};

// =============================================================================
// System Variants (for hot-swapping)
// =============================================================================

/// @brief Variant type for system implementations
enum class SystemVariant : u8
{
    Scalar,       // Basic scalar implementation
    SIMD,         // SIMD-optimized implementation
    GPU,          // GPU-accelerated implementation
    Approximate,  // Degraded/approximate implementation
};

/// @brief Convert variant to string
[[nodiscard]] inline constexpr const char* toString(SystemVariant variant) noexcept
{
    switch (variant) {
        case SystemVariant::Scalar:
            return "Scalar";
        case SystemVariant::SIMD:
            return "SIMD";
        case SystemVariant::GPU:
            return "GPU";
        case SystemVariant::Approximate:
            return "Approximate";
    }
    return "Unknown";
}

/// @brief System with multiple implementations that can be hot-swapped
class IVariantSystem
{
public:
    virtual ~IVariantSystem() = default;

    /// @brief Get available variants
    [[nodiscard]] virtual std::vector<SystemVariant> availableVariants() const = 0;

    /// @brief Get current variant
    [[nodiscard]] virtual SystemVariant currentVariant() const noexcept = 0;

    /// @brief Switch to a different variant
    /// @return true if switch was successful
    virtual bool switchVariant(SystemVariant variant) = 0;
};

// =============================================================================
// System Registry
// =============================================================================

/// @brief Manages system registration and execution order
class SystemRegistry
{
public:
    /// @brief Register a system
    template <typename T, typename... Args> T& registerSystem(Args&&... args)
    {
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *system;
        systems_.push_back(std::move(system));
        return ref;
    }

    /// @brief Get a system by type
    template <typename T> [[nodiscard]] T* getSystem()
    {
        TypeId id = typeId<T>();
        for (auto& system : systems_) {
            if (system->systemId() == id) {
                return static_cast<T*>(system.get());
            }
        }
        return nullptr;
    }

    /// @brief Replace an existing system with a new one
    /// @tparam T The type of the system to replace
    /// @tparam NewT The type of the new system implementation
    /// @param args Arguments for the new system constructor
    /// @return Reference to the new system
    template <typename T, typename NewT, typename... Args>
    NewT& replaceSystem(World& world, Args&&... args)
    {
        TypeId id = typeId<T>();
        for (auto it = systems_.begin(); it != systems_.end(); ++it) {
            if ((*it)->systemId() == id) {
                // Shutdown old system
                (*it)->shutdown(world);

                // Create new system
                auto newSystem = std::make_unique<NewT>(std::forward<Args>(args)...);
                NewT& ref = *newSystem;

                // Replace in registry
                *it = std::move(newSystem);

                // Initialize new system
                ref.init(world);

                return ref;
            }
        }

        // If not found, just register as new
        return registerSystem<NewT>(std::forward<Args>(args)...);
    }

    /// @brief Replace an existing system by name with a new one
    /// @tparam NewT The type of the new system implementation
    /// @param world The ECS world
    /// @param name The name of the system to replace
    /// @param args Arguments for the new system constructor
    /// @return Reference to the new system
    template <typename NewT, typename... Args>
    NewT& replaceSystemByName(World& world, const char* name, Args&&... args)
    {
        for (auto it = systems_.begin(); it != systems_.end(); ++it) {
            if (std::string((*it)->name()) == name) {
                // Shutdown old system
                (*it)->shutdown(world);

                // Create new system
                auto newSystem = std::make_unique<NewT>(std::forward<Args>(args)...);
                NewT& ref = *newSystem;

                // Replace in registry
                *it = std::move(newSystem);

                // Initialize new system
                ref.init(world);

                return ref;
            }
        }

        // If not found, just register as new
        return registerSystem<NewT>(std::forward<Args>(args)...);
    }

    /// @brief Initialize all systems
    void initAll(World& world)
    {
        for (auto& system : systems_) {
            system->init(world);
        }
    }

    /// @brief Update all enabled systems
    void updateAll(World& world, f32 dt)
    {
        for (auto& system : systems_) {
            if (system->isEnabled()) {
                system->update(world, dt);
            }
        }
    }

    /// @brief Shutdown all systems
    void shutdownAll(World& world)
    {
        // Shutdown in reverse order
        for (auto it = systems_.rbegin(); it != systems_.rend(); ++it) {
            (*it)->shutdown(world);
        }
    }

    /// @brief Get all systems
    [[nodiscard]] const std::vector<std::unique_ptr<ISystem>>& systems() const { return systems_; }

    /// @brief Get number of systems
    [[nodiscard]] usize count() const noexcept { return systems_.size(); }

    /// @brief Clear all systems
    void clear() { systems_.clear(); }

private:
    std::vector<std::unique_ptr<ISystem>> systems_;
};

}  // namespace autophage::ecs
