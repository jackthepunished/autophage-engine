#pragma once

/// @file components.hpp
/// @brief Common ECS components for Autophage Engine

#include <autophage/core/platform.hpp>
#include <autophage/core/types.hpp>
#include <autophage/ecs/entity.hpp>

#include <cmath>

namespace autophage::ecs {

// =============================================================================
// Math Types (minimal, for components)
// =============================================================================

/// @brief 3D vector (aligned for SIMD)
struct AUTOPHAGE_ALIGN(16) Vec3
{
    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 z = 0.0f;
    f32 _pad = 0.0f;  // Padding for 16-byte alignment

    constexpr Vec3() = default;
    constexpr Vec3(f32 x_, f32 y_, f32 z_) : x(x_), y(y_), z(z_) {}

    [[nodiscard]] constexpr Vec3 operator+(const Vec3& other) const noexcept
    {
        return {x + other.x, y + other.y, z + other.z};
    }

    [[nodiscard]] constexpr Vec3 operator-(const Vec3& other) const noexcept
    {
        return {x - other.x, y - other.y, z - other.z};
    }

    [[nodiscard]] constexpr Vec3 operator*(f32 scalar) const noexcept
    {
        return {x * scalar, y * scalar, z * scalar};
    }

    constexpr Vec3& operator+=(const Vec3& other) noexcept
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    constexpr Vec3& operator-=(const Vec3& other) noexcept
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    constexpr Vec3& operator*=(f32 scalar) noexcept
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    [[nodiscard]] f32 length() const noexcept { return std::sqrt(x * x + y * y + z * z); }

    [[nodiscard]] f32 lengthSquared() const noexcept { return x * x + y * y + z * z; }

    [[nodiscard]] Vec3 normalized() const noexcept
    {
        f32 len = length();
        if (len > 0.0f) {
            return *this * (1.0f / len);
        }
        return *this;
    }

    [[nodiscard]] static constexpr Vec3 zero() noexcept { return {0, 0, 0}; }
    [[nodiscard]] static constexpr Vec3 one() noexcept { return {1, 1, 1}; }
    [[nodiscard]] static constexpr Vec3 up() noexcept { return {0, 1, 0}; }
    [[nodiscard]] static constexpr Vec3 forward() noexcept { return {0, 0, 1}; }
    [[nodiscard]] static constexpr Vec3 right() noexcept { return {1, 0, 0}; }
};

/// @brief Quaternion for rotations (aligned for SIMD)
struct AUTOPHAGE_ALIGN(16) Quat
{
    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 z = 0.0f;
    f32 w = 1.0f;

    constexpr Quat() = default;
    constexpr Quat(f32 x_, f32 y_, f32 z_, f32 w_) : x(x_), y(y_), z(z_), w(w_) {}

    [[nodiscard]] static constexpr Quat identity() noexcept { return {0, 0, 0, 1}; }

    /// @brief Create from Euler angles (radians)
    [[nodiscard]] static Quat fromEuler(f32 pitch, f32 yaw, f32 roll) noexcept
    {
        f32 cy = std::cos(yaw * 0.5f);
        f32 sy = std::sin(yaw * 0.5f);
        f32 cp = std::cos(pitch * 0.5f);
        f32 sp = std::sin(pitch * 0.5f);
        f32 cr = std::cos(roll * 0.5f);
        f32 sr = std::sin(roll * 0.5f);

        return {sr * cp * cy - cr * sp * sy, cr * sp * cy + sr * cp * sy,
                cr * cp * sy - sr * sp * cy, cr * cp * cy + sr * sp * sy};
    }
};

// =============================================================================
// Transform Component
// =============================================================================

/// @brief Transform component - position, rotation, scale
struct AUTOPHAGE_ALIGN(64) Transform
{
    Vec3 position{0, 0, 0};
    Quat rotation{0, 0, 0, 1};
    Vec3 scale{1, 1, 1};

    constexpr Transform() = default;
    constexpr Transform(Vec3 pos) : position(pos) {}
    constexpr Transform(Vec3 pos, Quat rot) : position(pos), rotation(rot) {}
    constexpr Transform(Vec3 pos, Quat rot, Vec3 scl) : position(pos), rotation(rot), scale(scl) {}
};

// =============================================================================
// Velocity Component
// =============================================================================

/// @brief Velocity component - linear and angular velocity
struct AUTOPHAGE_ALIGN(32) Velocity
{
    Vec3 linear{0, 0, 0};
    Vec3 angular{0, 0, 0};

    constexpr Velocity() = default;
    constexpr Velocity(Vec3 lin) : linear(lin) {}
    constexpr Velocity(Vec3 lin, Vec3 ang) : linear(lin), angular(ang) {}
};

// =============================================================================
// Hierarchy Component
// =============================================================================

/// @brief Hierarchy component for parent-child relationships
struct Hierarchy
{
    Entity parent{0, 0};
    Entity firstChild{0, 0};
    Entity nextSibling{0, 0};
    Entity prevSibling{0, 0};
    u32 depth = 0;

    [[nodiscard]] bool hasParent() const noexcept { return parent.isValid(); }

    [[nodiscard]] bool hasChildren() const noexcept { return firstChild.isValid(); }
};

// =============================================================================
// Common Tags (zero-size components)
// =============================================================================

/// @brief Tag: Entity is active and should be processed
struct Active
{};

/// @brief Tag: Entity is static and won't move
struct Static
{};

/// @brief Tag: Entity is dirty and needs recalculation
struct Dirty
{};

/// @brief Tag: Entity is visible and should be rendered
struct Visible
{};

/// @brief Tag: Entity is marked for destruction
struct Destroyed
{};

// =============================================================================
// Physics Components
// =============================================================================

/// @brief Mass component for physics
struct Mass
{
    f32 value = 1.0f;
    f32 inverseMass = 1.0f;

    constexpr Mass() = default;
    explicit constexpr Mass(f32 m) : value(m), inverseMass(m > 0 ? 1.0f / m : 0.0f) {}
};

/// @brief Acceleration component
struct AUTOPHAGE_ALIGN(16) Acceleration
{
    Vec3 value{0, 0, 0};

    constexpr Acceleration() = default;
    explicit constexpr Acceleration(Vec3 v) : value(v) {}
};

/// @brief Gravity component (per-entity gravity override)
struct AUTOPHAGE_ALIGN(16) Gravity
{
    Vec3 value{0, -9.81f, 0};

    constexpr Gravity() = default;
    explicit constexpr Gravity(Vec3 v) : value(v) {}
};

// =============================================================================
// Bounds Components
// =============================================================================

/// @brief Axis-aligned bounding box
struct AUTOPHAGE_ALIGN(32) AABB
{
    Vec3 min{0, 0, 0};
    Vec3 max{0, 0, 0};

    constexpr AABB() = default;
    constexpr AABB(Vec3 min_, Vec3 max_) : min(min_), max(max_) {}

    [[nodiscard]] Vec3 center() const noexcept
    {
        return Vec3{(min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f, (min.z + max.z) * 0.5f};
    }

    [[nodiscard]] Vec3 extents() const noexcept
    {
        return Vec3{(max.x - min.x) * 0.5f, (max.y - min.y) * 0.5f, (max.z - min.z) * 0.5f};
    }
};

/// @brief Bounding sphere
struct BoundingSphere
{
    Vec3 center{0, 0, 0};
    f32 radius = 0.0f;

    constexpr BoundingSphere() = default;
    constexpr BoundingSphere(Vec3 c, f32 r) : center(c), radius(r) {}
};

// =============================================================================
// Rendering Components
// =============================================================================

/// @brief Renderable component (basic color/shape info)
struct Renderable
{
    u8 r = 255;
    u8 g = 255;
    u8 b = 255;
    u8 a = 255;

    constexpr Renderable() = default;
    constexpr Renderable(u8 r_, u8 g_, u8 b_, u8 a_ = 255) : r(r_), g(g_), b(b_), a(a_) {}
};

}  // namespace autophage::ecs
