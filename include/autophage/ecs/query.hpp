#pragma once

/// @file query.hpp
/// @brief Query system for iterating entities with specific components

#include <autophage/core/types.hpp>
#include <autophage/ecs/entity.hpp>
#include <autophage/ecs/component_storage.hpp>

#include <tuple>
#include <vector>

namespace autophage::ecs {

// =============================================================================
// Query - Iterate entities with specific components
// =============================================================================

/// @brief Query for entities with specific components
/// @tparam Components Component types to query
template <Component... Components>
class Query {
public:
    explicit Query(ComponentRegistry& registry)
        : arrays_(std::make_tuple(&registry.getArray<Components>()...)) {}

    /// @brief Iterate over all entities matching the query
    /// @param func Function to call with (Entity, Component&...)
    template <typename Func>
    void forEach(Func&& func) {
        // Get the smallest array to iterate (optimization)
        auto& primary = *std::get<0>(arrays_);

        primary.forEach([&](Entity entity, auto& /*unused*/) {
            if (matchesAll(entity)) {
                func(entity, *std::get<ComponentArray<Components>*>(arrays_)->get(entity)...);
            }
        });
    }

    /// @brief Iterate over all entities matching the query (const)
    template <typename Func>
    void forEach(Func&& func) const {
        const auto& primary = *std::get<0>(arrays_);

        primary.forEach([&](Entity entity, const auto& /*unused*/) {
            if (matchesAll(entity)) {
                func(entity, *std::get<ComponentArray<Components>*>(arrays_)->get(entity)...);
            }
        });
    }

    /// @brief Get all entities matching the query
    [[nodiscard]] std::vector<Entity> entities() const {
        std::vector<Entity> result;
        const auto& primary = *std::get<0>(arrays_);

        primary.forEach([&](Entity entity, const auto& /*unused*/) {
            if (matchesAll(entity)) {
                result.push_back(entity);
            }
        });

        return result;
    }

    /// @brief Count entities matching the query
    [[nodiscard]] usize count() const {
        usize result = 0;
        const auto& primary = *std::get<0>(arrays_);

        primary.forEach([&](Entity entity, const auto& /*unused*/) {
            if (matchesAll(entity)) {
                ++result;
            }
        });

        return result;
    }

    /// @brief Check if any entity matches the query
    [[nodiscard]] bool any() const {
        const auto& primary = *std::get<0>(arrays_);
        bool found = false;

        primary.forEach([&](Entity entity, const auto& /*unused*/) {
            if (!found && matchesAll(entity)) {
                found = true;
            }
        });

        return found;
    }

private:
    /// @brief Check if entity has all required components
    [[nodiscard]] bool matchesAll(Entity entity) const {
        return (std::get<ComponentArray<Components>*>(arrays_)->has(entity) && ...);
    }

    std::tuple<ComponentArray<Components>*...> arrays_;
};

// =============================================================================
// View - Lightweight iteration without allocation
// =============================================================================

/// @brief Lightweight view for iterating entities with components
/// Does not allocate, iterates directly over component arrays
template <Component... Components>
class View {
public:
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::tuple<Entity, Components&...>;

        Iterator(View* view, usize index) : view_(view), index_(index) {
            skipInvalid();
        }

        value_type operator*() {
            Entity entity = view_->primaryEntities_[index_];
            return std::make_tuple(
                entity,
                std::ref(*std::get<ComponentArray<Components>*>(view_->arrays_)->get(entity))...
            );
        }

        Iterator& operator++() {
            ++index_;
            skipInvalid();
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return index_ != other.index_;
        }

    private:
        void skipInvalid() {
            while (index_ < view_->primaryEntities_.size()) {
                Entity entity = view_->primaryEntities_[index_];
                if (view_->matchesAll(entity)) {
                    break;
                }
                ++index_;
            }
        }

        View* view_;
        usize index_;
    };

    explicit View(ComponentRegistry& registry)
        : arrays_(std::make_tuple(&registry.getArray<Components>()...)) {
        // Cache entities from primary array
        auto& primary = *std::get<0>(arrays_);
        primaryEntities_ = primary.entities();
    }

    Iterator begin() { return Iterator(this, 0); }
    Iterator end() { return Iterator(this, primaryEntities_.size()); }

private:
    friend struct Iterator;

    [[nodiscard]] bool matchesAll(Entity entity) const {
        return (std::get<ComponentArray<Components>*>(arrays_)->has(entity) && ...);
    }

    std::tuple<ComponentArray<Components>*...> arrays_;
    std::vector<Entity> primaryEntities_;
};

}  // namespace autophage::ecs
