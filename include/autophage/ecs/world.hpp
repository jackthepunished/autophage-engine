#pragma once

/// @file world.hpp
/// @brief ECS World - main container for entities, components, and systems

#include <autophage/core/types.hpp>
#include <autophage/ecs/component_storage.hpp>
#include <autophage/ecs/entity.hpp>
#include <autophage/ecs/query.hpp>
#include <autophage/ecs/system.hpp>

namespace autophage::ecs {

// =============================================================================
// World - Main ECS Container
// =============================================================================

/// @brief The main ECS world containing entities, components, and systems
class World
{
public:
    World() = default;
    ~World() = default;

    // Non-copyable, moveable
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    World(World&&) noexcept = default;
    World& operator=(World&&) noexcept = default;

    // =========================================================================
    // Entity Management
    // =========================================================================

    /// @brief Create a new entity
    [[nodiscard]] Entity createEntity() { return entities_.create(); }

    /// @brief Destroy an entity and all its components
    void destroyEntity(Entity entity)
    {
        if (entities_.destroy(entity)) {
            components_.onEntityDestroyed(entity);
        }
    }

    /// @brief Check if an entity is alive
    [[nodiscard]] bool isAlive(Entity entity) const noexcept { return entities_.isAlive(entity); }

    /// @brief Get the number of alive entities
    [[nodiscard]] usize entityCount() const noexcept { return entities_.aliveCount(); }

    /// @brief Reserve capacity for entities
    void reserveEntities(usize count) { entities_.reserve(count); }

    // =========================================================================
    // Component Management
    // =========================================================================

    /// @brief Add a component to an entity
    template <Component T> T& addComponent(Entity entity, T component = T{})
    {
        return components_.getArray<T>().set(entity, std::move(component));
    }

    /// @brief Get a component from an entity (mutable)
    template <Component T> [[nodiscard]] T* getComponent(Entity entity)
    {
        return components_.getArray<T>().get(entity);
    }

    /// @brief Get a component from an entity (const)
    template <Component T> [[nodiscard]] const T* getComponent(Entity entity) const
    {
        return components_.getArray<T>().get(entity);
    }

    /// @brief Check if an entity has a component
    template <Component T> [[nodiscard]] bool hasComponent(Entity entity) const
    {
        return components_.getArray<T>().has(entity);
    }

    /// @brief Remove a component from an entity
    template <Component T> void removeComponent(Entity entity)
    {
        components_.getArray<T>().remove(entity);
    }

    /// @brief Register a component type
    template <Component T> void registerComponent() { components_.registerComponent<T>(); }

    // =========================================================================
    // Query
    // =========================================================================

    /// @brief Create a query for entities with specific components
    template <Component... Components> [[nodiscard]] Query<Components...> query()
    {
        return Query<Components...>(components_);
    }

    /// @brief Create a view for iterating entities with specific components
    template <Component... Components> [[nodiscard]] View<Components...> view()
    {
        return View<Components...>(components_);
    }

    // =========================================================================
    // System Management
    // =========================================================================

    /// @brief Register a system
    template <typename T, typename... Args> T& registerSystem(Args&&... args)
    {
        return systems_.registerSystem<T>(std::forward<Args>(args)...);
    }

    /// @brief Get a system by type
    template <typename T> [[nodiscard]] T* getSystem() { return systems_.getSystem<T>(); }

    /// @brief Replace an existing system with a new one
    template <typename T, typename NewT, typename... Args> NewT& replaceSystem(Args&&... args)
    {
        return systems_.replaceSystem<T, NewT>(*this, std::forward<Args>(args)...);
    }

    /// @brief Initialize all systems
    void initSystems() { systems_.initAll(*this); }

    /// @brief Update all systems
    void updateSystems(f32 dt) { systems_.updateAll(*this, dt); }

    /// @brief Shutdown all systems
    void shutdownSystems() { systems_.shutdownAll(*this); }

    /// @brief Get the system registry
    [[nodiscard]] SystemRegistry& systemRegistry() { return systems_; }
    [[nodiscard]] const SystemRegistry& systemRegistry() const { return systems_; }

    // =========================================================================
    // Lifecycle
    // =========================================================================

    /// @brief Initialize the world
    void init() { initSystems(); }

    /// @brief Update the world
    void update(f32 dt) { updateSystems(dt); }

    /// @brief Shutdown the world
    void shutdown() { shutdownSystems(); }

    /// @brief Clear all entities and components
    void clear()
    {
        entities_.clear();
        components_.clear();
    }

    // =========================================================================
    // Accessors
    // =========================================================================

    [[nodiscard]] EntityManager& entityManager() { return entities_; }
    [[nodiscard]] const EntityManager& entityManager() const { return entities_; }

    [[nodiscard]] ComponentRegistry& componentRegistry() { return components_; }
    [[nodiscard]] const ComponentRegistry& componentRegistry() const { return components_; }

private:
    EntityManager entities_;
    ComponentRegistry components_;
    SystemRegistry systems_;
};

// =============================================================================
// Entity Builder (fluent API)
// =============================================================================

/// @brief Fluent builder for creating entities with components
class EntityBuilder
{
public:
    EntityBuilder(World& world, Entity entity) : world_(world), entity_(entity) {}

    /// @brief Add a component to the entity
    template <Component T> EntityBuilder& with(T component = T{})
    {
        world_.addComponent<T>(entity_, std::move(component));
        return *this;
    }

    /// @brief Get the built entity
    [[nodiscard]] Entity build() const { return entity_; }

    /// @brief Implicit conversion to Entity
    operator Entity() const { return entity_; }

private:
    World& world_;
    Entity entity_;
};

/// @brief Create an entity builder
[[nodiscard]] inline EntityBuilder createEntity(World& world)
{
    return EntityBuilder(world, world.createEntity());
}

}  // namespace autophage::ecs
