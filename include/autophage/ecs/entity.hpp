#pragma once

/// @file entity.hpp
/// @brief Entity management for ECS

#include <autophage/core/types.hpp>

#include <vector>

namespace autophage::ecs {

// =============================================================================
// Entity Manager
// =============================================================================

/// @brief Manages entity creation, destruction, and recycling
class EntityManager
{
public:
    EntityManager() = default;
    ~EntityManager() = default;

    // Non-copyable, moveable
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;
    EntityManager(EntityManager&&) noexcept = default;
    EntityManager& operator=(EntityManager&&) noexcept = default;

    /// @brief Create a new entity
    /// @return The newly created entity
    [[nodiscard]] Entity create()
    {
        if (!freeList_.empty()) {
            // Recycle an old entity slot
            u32 index = freeList_.back();
            freeList_.pop_back();
            // Increment generation to invalidate old references
            generations_[index]++;
            alive_[index] = true;
            ++aliveCount_;
            return Entity{index, generations_[index]};
        }

        // Allocate new slot
        u32 index = static_cast<u32>(generations_.size());
        generations_.push_back(1);  // Start at generation 1 (0 is invalid)
        alive_.push_back(true);
        ++aliveCount_;
        return Entity{index, 1};
    }

    /// @brief Destroy an entity
    /// @param entity The entity to destroy
    /// @return true if the entity was destroyed, false if it was invalid
    bool destroy(Entity entity)
    {
        if (!isAlive(entity)) {
            return false;
        }

        alive_[entity.index] = false;
        freeList_.push_back(entity.index);
        --aliveCount_;
        return true;
    }

    /// @brief Check if an entity is alive
    /// @param entity The entity to check
    /// @return true if the entity exists and is alive
    [[nodiscard]] bool isAlive(Entity entity) const noexcept
    {
        if (entity.index >= generations_.size()) {
            return false;
        }
        return alive_[entity.index] && generations_[entity.index] == entity.generation;
    }

    /// @brief Get the number of alive entities
    [[nodiscard]] usize aliveCount() const noexcept { return aliveCount_; }

    /// @brief Get the total capacity (including recycled slots)
    [[nodiscard]] usize capacity() const noexcept { return generations_.size(); }

    /// @brief Get the number of entities that can be recycled
    [[nodiscard]] usize recycledCount() const noexcept { return freeList_.size(); }

    /// @brief Reserve capacity for entities
    void reserve(usize count)
    {
        generations_.reserve(count);
        alive_.reserve(count);
    }

    /// @brief Clear all entities (resets the manager)
    void clear()
    {
        generations_.clear();
        alive_.clear();
        freeList_.clear();
        aliveCount_ = 0;
    }

    /// @brief Iterate over all alive entities
    template <typename Func> void forEach(Func&& func) const
    {
        for (usize i = 0; i < alive_.size(); ++i) {
            if (alive_[i]) {
                func(Entity{static_cast<u32>(i), generations_[i]});
            }
        }
    }

private:
    std::vector<u32> generations_;  // Generation per slot
    std::vector<bool> alive_;       // Is slot currently in use
    std::vector<u32> freeList_;     // Recycled slots
    usize aliveCount_ = 0;
};

}  // namespace autophage::ecs

// Hash support for Entity
template <> struct std::hash<autophage::ecs::Entity>
{
    [[nodiscard]] size_t operator()(const autophage::ecs::Entity& e) const noexcept
    {
        return std::hash<uint64_t>{}((static_cast<uint64_t>(e.index) << 32) | e.generation);
    }
};
