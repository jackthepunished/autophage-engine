#pragma once

/// @file component_storage.hpp
/// @brief Component storage implementations for ECS (SoA and AoS)

#include <autophage/core/types.hpp>
#include <autophage/core/type_id.hpp>
#include <autophage/core/memory.hpp>
#include <autophage/ecs/entity.hpp>

#include <memory>
#include <unordered_map>
#include <vector>

namespace autophage::ecs {

// =============================================================================
// Component Array Interface
// =============================================================================

/// @brief Abstract base for type-erased component storage
class IComponentArray {
public:
    virtual ~IComponentArray() = default;

    /// @brief Get the TypeId of the stored component
    [[nodiscard]] virtual TypeId componentType() const noexcept = 0;

    /// @brief Get the number of components stored
    [[nodiscard]] virtual usize size() const noexcept = 0;

    /// @brief Check if entity has this component
    [[nodiscard]] virtual bool has(Entity entity) const noexcept = 0;

    /// @brief Remove component from entity
    virtual void remove(Entity entity) = 0;

    /// @brief Called when an entity is destroyed
    virtual void onEntityDestroyed(Entity entity) = 0;

    /// @brief Get raw pointer to component data (for entity)
    [[nodiscard]] virtual void* getRaw(Entity entity) = 0;
    [[nodiscard]] virtual const void* getRaw(Entity entity) const = 0;
};

// =============================================================================
// Sparse Set Component Storage
// =============================================================================

/// @brief Sparse set based component storage
/// Provides O(1) add/remove/lookup with cache-friendly iteration
template <Component T>
class ComponentArray : public IComponentArray {
public:
    ComponentArray() = default;

    [[nodiscard]] TypeId componentType() const noexcept override {
        return typeId<T>();
    }

    [[nodiscard]] usize size() const noexcept override {
        return dense_.size();
    }

    /// @brief Add or replace a component for an entity
    /// @return Reference to the component
    T& set(Entity entity, T component = T{}) {
        if (has(entity)) {
            // Replace existing
            usize denseIdx = sparse_[entity.index];
            dense_[denseIdx].component = std::move(component);
            return dense_[denseIdx].component;
        }

        // Ensure sparse array is large enough
        if (entity.index >= sparse_.size()) {
            sparse_.resize(entity.index + 1, INVALID_INDEX);
        }

        // Add new component
        sparse_[entity.index] = dense_.size();
        dense_.push_back({entity, std::move(component)});
        return dense_.back().component;
    }

    /// @brief Get component for entity (mutable)
    [[nodiscard]] T* get(Entity entity) {
        if (!has(entity)) return nullptr;
        return &dense_[sparse_[entity.index]].component;
    }

    /// @brief Get component for entity (const)
    [[nodiscard]] const T* get(Entity entity) const {
        if (!has(entity)) return nullptr;
        return &dense_[sparse_[entity.index]].component;
    }

    [[nodiscard]] void* getRaw(Entity entity) override {
        return get(entity);
    }

    [[nodiscard]] const void* getRaw(Entity entity) const override {
        return get(entity);
    }

    [[nodiscard]] bool has(Entity entity) const noexcept override {
        if (entity.index >= sparse_.size()) return false;
        usize denseIdx = sparse_[entity.index];
        if (denseIdx == INVALID_INDEX || denseIdx >= dense_.size()) return false;
        return dense_[denseIdx].entity == entity;
    }

    void remove(Entity entity) override {
        if (!has(entity)) return;

        usize denseIdx = sparse_[entity.index];
        usize lastIdx = dense_.size() - 1;

        if (denseIdx != lastIdx) {
            // Swap with last element
            dense_[denseIdx] = std::move(dense_[lastIdx]);
            sparse_[dense_[denseIdx].entity.index] = denseIdx;
        }

        dense_.pop_back();
        sparse_[entity.index] = INVALID_INDEX;
    }

    void onEntityDestroyed(Entity entity) override {
        remove(entity);
    }

    /// @brief Iterate over all components
    template <typename Func>
    void forEach(Func&& func) {
        for (auto& entry : dense_) {
            func(entry.entity, entry.component);
        }
    }

    /// @brief Iterate over all components (const)
    template <typename Func>
    void forEach(Func&& func) const {
        for (const auto& entry : dense_) {
            func(entry.entity, entry.component);
        }
    }

    /// @brief Get all entities with this component
    [[nodiscard]] std::vector<Entity> entities() const {
        std::vector<Entity> result;
        result.reserve(dense_.size());
        for (const auto& entry : dense_) {
            result.push_back(entry.entity);
        }
        return result;
    }

    /// @brief Direct access to dense component data (for SIMD operations)
    [[nodiscard]] T* data() noexcept {
        // Note: This only works if we store components contiguously
        // For now, we return nullptr as components are interleaved with entities
        return nullptr;
    }

    /// @brief Reserve capacity
    void reserve(usize count) {
        dense_.reserve(count);
    }

    /// @brief Clear all components
    void clear() {
        dense_.clear();
        sparse_.clear();
    }

private:
    static constexpr usize INVALID_INDEX = ~usize{0};

    struct Entry {
        Entity entity;
        T component;
    };

    std::vector<Entry> dense_;    // Packed component data
    std::vector<usize> sparse_;   // Entity index -> dense index
};

// =============================================================================
// SoA Component Storage (for hot data)
// =============================================================================

/// @brief Structure-of-Arrays storage for better cache performance
/// Stores each field separately for SIMD-friendly access
template <Component T>
class ComponentArraySoA : public IComponentArray {
    // TODO: Implement SoA storage with reflection/structured bindings
    // For now, this is a placeholder that falls back to AoS
public:
    ComponentArraySoA() = default;

    [[nodiscard]] TypeId componentType() const noexcept override {
        return typeId<T>();
    }

    [[nodiscard]] usize size() const noexcept override {
        return inner_.size();
    }

    [[nodiscard]] bool has(Entity entity) const noexcept override {
        return inner_.has(entity);
    }

    void remove(Entity entity) override {
        inner_.remove(entity);
    }

    void onEntityDestroyed(Entity entity) override {
        inner_.onEntityDestroyed(entity);
    }

    [[nodiscard]] void* getRaw(Entity entity) override {
        return inner_.getRaw(entity);
    }

    [[nodiscard]] const void* getRaw(Entity entity) const override {
        return inner_.getRaw(entity);
    }

    T& set(Entity entity, T component = T{}) {
        return inner_.set(entity, std::move(component));
    }

    [[nodiscard]] T* get(Entity entity) {
        return inner_.get(entity);
    }

    [[nodiscard]] const T* get(Entity entity) const {
        return inner_.get(entity);
    }

private:
    ComponentArray<T> inner_;  // Fallback to AoS for now
};

// =============================================================================
// Component Registry
// =============================================================================

/// @brief Manages all component arrays
class ComponentRegistry {
public:
    ComponentRegistry() = default;

    /// @brief Register a component type
    template <Component T>
    void registerComponent() {
        TypeId id = typeId<T>();
        if (arrays_.find(id) == arrays_.end()) {
            arrays_[id] = std::make_unique<ComponentArray<T>>();
        }
    }

    /// @brief Get or create a component array
    template <Component T>
    ComponentArray<T>& getArray() {
        TypeId id = typeId<T>();
        auto it = arrays_.find(id);
        if (it == arrays_.end()) {
            registerComponent<T>();
            it = arrays_.find(id);
        }
        return static_cast<ComponentArray<T>&>(*it->second);
    }

    /// @brief Check if a component type is registered
    template <Component T>
    [[nodiscard]] bool isRegistered() const {
        return arrays_.find(typeId<T>()) != arrays_.end();
    }

    /// @brief Notify all arrays that an entity was destroyed
    void onEntityDestroyed(Entity entity) {
        for (auto& [id, array] : arrays_) {
            array->onEntityDestroyed(entity);
        }
    }

    /// @brief Get component array by TypeId (type-erased)
    [[nodiscard]] IComponentArray* getArrayById(TypeId id) {
        auto it = arrays_.find(id);
        return it != arrays_.end() ? it->second.get() : nullptr;
    }

    /// @brief Clear all components
    void clear() {
        arrays_.clear();
    }

private:
    std::unordered_map<TypeId, std::unique_ptr<IComponentArray>> arrays_;
};

}  // namespace autophage::ecs
