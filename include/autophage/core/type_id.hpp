#pragma once

/// @file type_id.hpp
/// @brief Compile-time type identification system for ECS

#include <autophage/core/types.hpp>

#include <type_traits>

namespace autophage {

// =============================================================================
// Compile-Time String Hash (FNV-1a)
// =============================================================================

namespace detail {

/// @brief FNV-1a hash constants
struct FnvHash {
    static constexpr u64 OFFSET_BASIS = 14695981039346656037ULL;
    static constexpr u64 PRIME = 1099511628211ULL;
};

/// @brief Compile-time FNV-1a hash for strings
[[nodiscard]] constexpr u64 fnv1aHash(const char* str, u64 hash = FnvHash::OFFSET_BASIS) noexcept {
    return (*str == '\0') ? hash : fnv1aHash(str + 1, (hash ^ static_cast<u64>(*str)) * FnvHash::PRIME);
}

/// @brief Compile-time hash combining
[[nodiscard]] constexpr u64 hashCombine(u64 h1, u64 h2) noexcept {
    return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
}

// =============================================================================
// Type Name Extraction (Compiler-specific)
// =============================================================================

/// @brief Get type name at compile time
/// Uses __PRETTY_FUNCTION__ (GCC/Clang) or __FUNCSIG__ (MSVC)
template <typename T>
[[nodiscard]] constexpr const char* rawTypeName() noexcept {
#if defined(__clang__) || defined(__GNUC__)
    return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
    return __FUNCSIG__;
#else
    return "unknown";
#endif
}

/// @brief Hash a type name at compile time
template <typename T>
[[nodiscard]] constexpr u64 typeNameHash() noexcept {
    return fnv1aHash(rawTypeName<T>());
}

}  // namespace detail

// =============================================================================
// TypeId - Main Interface
// =============================================================================

/// @brief Type identifier for runtime type operations
/// Uses compile-time hashing for zero-cost type identification
class TypeId {
public:
    constexpr TypeId() noexcept = default;
    
    explicit constexpr TypeId(u64 id) noexcept : id_(id) {}

    /// @brief Get the raw ID value
    [[nodiscard]] constexpr u64 value() const noexcept { return id_; }

    /// @brief Check if this is a valid (non-zero) type ID
    [[nodiscard]] constexpr bool isValid() const noexcept { return id_ != 0; }

    /// @brief Comparison operators
    [[nodiscard]] constexpr bool operator==(const TypeId& other) const noexcept {
        return id_ == other.id_;
    }

    [[nodiscard]] constexpr bool operator!=(const TypeId& other) const noexcept {
        return id_ != other.id_;
    }

    [[nodiscard]] constexpr bool operator<(const TypeId& other) const noexcept {
        return id_ < other.id_;
    }

    [[nodiscard]] constexpr bool operator>(const TypeId& other) const noexcept {
        return id_ > other.id_;
    }

    [[nodiscard]] constexpr bool operator<=(const TypeId& other) const noexcept {
        return id_ <= other.id_;
    }

    [[nodiscard]] constexpr bool operator>=(const TypeId& other) const noexcept {
        return id_ >= other.id_;
    }

private:
    u64 id_ = 0;
};

/// @brief Invalid type ID constant
inline constexpr TypeId INVALID_TYPE_ID{0};

// =============================================================================
// Type Traits and Helpers
// =============================================================================

/// @brief Get TypeId for a type at compile time
/// @tparam T The type to get the ID for
template <typename T>
[[nodiscard]] constexpr TypeId typeId() noexcept {
    return TypeId{detail::typeNameHash<std::remove_cvref_t<T>>()};
}

/// @brief Get TypeId from a runtime instance
/// @tparam T The type of the instance
template <typename T>
[[nodiscard]] constexpr TypeId typeIdOf([[maybe_unused]] const T& instance) noexcept {
    return typeId<T>();
}

// =============================================================================
// Component Type Traits
// =============================================================================

/// @brief Concept for valid ECS components
template <typename T>
concept Component = std::is_trivially_copyable_v<T> &&
                    std::is_trivially_destructible_v<T> &&
                    std::is_default_constructible_v<T> &&
                    (alignof(T) <= 64);  // Max cache-line alignment

/// @brief Concept for tag components (zero-size)
template <typename T>
concept TagComponent = Component<T> && std::is_empty_v<T>;

/// @brief Type information for components
template <typename T>
struct TypeInfo {
    static constexpr TypeId id = typeId<T>();
    static constexpr usize size = sizeof(T);
    static constexpr usize alignment = alignof(T);
    static constexpr bool isTrivial = std::is_trivially_copyable_v<T>;
    static constexpr bool isTag = std::is_empty_v<T>;
};

// =============================================================================
// Type List Utilities (for archetype matching)
// =============================================================================

/// @brief Compile-time type list
template <typename... Ts>
struct TypeList {
    static constexpr usize size = sizeof...(Ts);
};

/// @brief Check if a type is in a type list
template <typename T, typename List>
struct TypeListContains;

template <typename T, typename... Ts>
struct TypeListContains<T, TypeList<Ts...>> {
    static constexpr bool value = (std::is_same_v<T, Ts> || ...);
};

template <typename T, typename List>
inline constexpr bool typeListContains = TypeListContains<T, List>::value;

/// @brief Get sorted TypeId array from type list (for archetype matching)
template <typename... Ts>
[[nodiscard]] constexpr auto typeIdArray() noexcept {
    std::array<TypeId, sizeof...(Ts)> ids{typeId<Ts>()...};
    // Note: Sorting would be done at runtime or with more complex constexpr
    return ids;
}

}  // namespace autophage

// =============================================================================
// Standard Library Hash Support
// =============================================================================

template <>
struct std::hash<autophage::TypeId> {
    [[nodiscard]] constexpr size_t operator()(const autophage::TypeId& id) const noexcept {
        return static_cast<size_t>(id.value());
    }
};
