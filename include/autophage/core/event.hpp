#pragma once

/// @file event.hpp
/// @brief Event bus system for decoupled communication

#include <autophage/core/assert.hpp>
#include <autophage/core/type_id.hpp>
#include <autophage/core/types.hpp>

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>


namespace autophage {

// =============================================================================
// Event Base
// =============================================================================

/// @brief Concept for event types
template <typename T>
concept EventType = std::is_class_v<T> && !std::is_const_v<T>;

// =============================================================================
// Event Bus
// =============================================================================

class EventBus
{
public:
    using ListenerId = u64;

    EventBus() = default;

    /// @brief Subscribe to an event type
    /// @return Listener ID for unsubscription
    template <EventType E, typename Func> ListenerId subscribe(Func&& callback)
    {
        TypeId type = typeId<E>();
        std::lock_guard<std::mutex> lock(mutex_);

        auto& dispatcher = dispatchers_[type];
        if (!dispatcher) {
            dispatcher = std::make_unique<EventDispatcher<E>>();
        }

        auto* specificDispatcher = static_cast<EventDispatcher<E>*>(dispatcher.get());
        ListenerId id = nextListenerId_++;
        specificDispatcher->listeners.push_back({id, std::forward<Func>(callback)});
        return id;
    }

    /// @brief Unsubscribe from an event type
    template <EventType E> void unsubscribe(ListenerId id)
    {
        TypeId type = typeId<E>();
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = dispatchers_.find(type);
        if (it != dispatchers_.end()) {
            auto* specificDispatcher = static_cast<EventDispatcher<E>*>(it->second.get());
            auto& listeners = specificDispatcher->listeners;

            std::erase_if(listeners, [id](const auto& entry) { return entry.id == id; });
        }
    }

    /// @brief Publish an event immediately
    template <EventType E> void publish(const E& event)
    {
        TypeId type = typeId<E>();

        // Copy listener list to avoid deadlocks/invalidation during callback
        // if callback modifies subscriptions (rudimentary safety)
        std::vector<std::function<void(const E&)>> callbacks;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = dispatchers_.find(type);
            if (it != dispatchers_.end()) {
                auto* specificDispatcher = static_cast<EventDispatcher<E>*>(it->second.get());
                callbacks.reserve(specificDispatcher->listeners.size());
                for (const auto& entry : specificDispatcher->listeners) {
                    callbacks.push_back(entry.callback);
                }
            }
        }

        // Execute callbacks outside lock
        for (const auto& cb : callbacks) {
            cb(event);
        }
    }

private:
    struct IEventDispatcher
    {
        virtual ~IEventDispatcher() = default;
    };

    template <typename E> struct EventDispatcher : IEventDispatcher
    {
        struct Entry
        {
            ListenerId id;
            std::function<void(const E&)> callback;
        };
        std::vector<Entry> listeners;
    };

    std::unordered_map<TypeId, std::unique_ptr<IEventDispatcher>> dispatchers_;
    std::mutex mutex_;
    ListenerId nextListenerId_ = 1;
};

}  // namespace autophage
