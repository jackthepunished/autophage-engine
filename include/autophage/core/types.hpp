#pragma once

/// @file types.hpp
/// @brief Core type definitions and aliases for Autophage Engine

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

// C++23 features with fallback
#if __has_include(<expected>)
    #include <expected>
    #define AUTOPHAGE_HAS_EXPECTED 1
#else
    #define AUTOPHAGE_HAS_EXPECTED 0
#endif

#if __has_include(<span>)
    #include <span>
    #define AUTOPHAGE_HAS_SPAN 1
#else
    #define AUTOPHAGE_HAS_SPAN 0
#endif

namespace autophage {

// =============================================================================
// Integer Types
// =============================================================================

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using usize = std::size_t;
using isize = std::ptrdiff_t;

// =============================================================================
// Floating Point Types
// =============================================================================

using f32 = float;
using f64 = double;

// =============================================================================
// String Types
// =============================================================================

using String = std::string;
using StringView = std::string_view;

// =============================================================================
// Utility Types
// =============================================================================

/// @brief Byte type for raw memory operations
using Byte = std::byte;

/// @brief Span type for non-owning views
#if AUTOPHAGE_HAS_SPAN
template <typename T>
using Span = std::span<T>;
#endif

/// @brief Optional type
template <typename T>
using Optional = std::optional<T>;

// =============================================================================
// Error Handling
// =============================================================================

/// @brief Error codes for engine operations
enum class ErrorCode : u32 {
    Success = 0,
    InvalidArgument,
    OutOfMemory,
    NotFound,
    AlreadyExists,
    InvalidState,
    Timeout,
    NotImplemented,
    SystemError,
    ValidationFailed,
    RollbackRequired,
};

/// @brief Convert error code to string
[[nodiscard]] constexpr StringView toString(ErrorCode code) noexcept {
    switch (code) {
        case ErrorCode::Success: return "Success";
        case ErrorCode::InvalidArgument: return "InvalidArgument";
        case ErrorCode::OutOfMemory: return "OutOfMemory";
        case ErrorCode::NotFound: return "NotFound";
        case ErrorCode::AlreadyExists: return "AlreadyExists";
        case ErrorCode::InvalidState: return "InvalidState";
        case ErrorCode::Timeout: return "Timeout";
        case ErrorCode::NotImplemented: return "NotImplemented";
        case ErrorCode::SystemError: return "SystemError";
        case ErrorCode::ValidationFailed: return "ValidationFailed";
        case ErrorCode::RollbackRequired: return "RollbackRequired";
    }
    return "Unknown";
}

// =============================================================================
// Result Type (Expected or Fallback)
// =============================================================================

#if AUTOPHAGE_HAS_EXPECTED

template <typename T, typename E>
using Expected = std::expected<T, E>;

template <typename E>
using Unexpected = std::unexpected<E>;

template <typename T>
using Result = std::expected<T, ErrorCode>;

using VoidResult = std::expected<void, ErrorCode>;

#else

/// @brief Simple Result type fallback when std::expected is not available
template <typename T>
class Result {
public:
    Result(T value) : data_(std::move(value)) {}
    Result(ErrorCode error) : data_(error) {}

    [[nodiscard]] bool hasValue() const noexcept {
        return std::holds_alternative<T>(data_);
    }

    [[nodiscard]] explicit operator bool() const noexcept { return hasValue(); }

    [[nodiscard]] T& value() & { return std::get<T>(data_); }
    [[nodiscard]] const T& value() const& { return std::get<T>(data_); }
    [[nodiscard]] T&& value() && { return std::get<T>(std::move(data_)); }

    [[nodiscard]] ErrorCode error() const noexcept {
        return std::get<ErrorCode>(data_);
    }

    [[nodiscard]] T valueOr(T defaultValue) const {
        return hasValue() ? value() : defaultValue;
    }

private:
    std::variant<T, ErrorCode> data_;
};

/// @brief Void result specialization
class VoidResult {
public:
    VoidResult() : error_(ErrorCode::Success) {}
    VoidResult(ErrorCode error) : error_(error) {}

    [[nodiscard]] bool hasValue() const noexcept {
        return error_ == ErrorCode::Success;
    }

    [[nodiscard]] explicit operator bool() const noexcept { return hasValue(); }

    [[nodiscard]] ErrorCode error() const noexcept { return error_; }

private:
    ErrorCode error_;
};

#endif

// =============================================================================
// Engine-Specific Types
// =============================================================================

/// @brief Entity ID type (generational index)
struct EntityId {
    u32 index = 0;
    u32 generation = 0;

    [[nodiscard]] constexpr bool isValid() const noexcept {
        return generation != 0;
    }

    [[nodiscard]] constexpr bool operator==(const EntityId& other) const noexcept {
        return index == other.index && generation == other.generation;
    }

    [[nodiscard]] constexpr bool operator!=(const EntityId& other) const noexcept {
        return !(*this == other);
    }

    [[nodiscard]] constexpr bool operator<(const EntityId& other) const noexcept {
        if (generation != other.generation) return generation < other.generation;
        return index < other.index;
    }
};

/// @brief Invalid entity constant
inline constexpr EntityId INVALID_ENTITY{0, 0};

/// @brief Component type ID (compile-time hash)
using ComponentTypeId = u64;

/// @brief System type ID
using SystemTypeId = u64;

/// @brief Frame number type
using FrameNumber = u64;

/// @brief Time in seconds (high precision)
using Seconds = f64;

/// @brief Time in milliseconds
using Milliseconds = f64;

/// @brief Time in microseconds
using Microseconds = f64;

}  // namespace autophage
