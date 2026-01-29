#pragma once

/// @file systems.hpp
/// @brief Common ECS systems with scalar and SIMD implementations

#include <autophage/core/platform.hpp>
#include <autophage/core/types.hpp>
#include <autophage/ecs/components.hpp>
#include <autophage/ecs/system.hpp>
#include <autophage/ecs/world.hpp>

// SIMD headers
#if defined(AUTOPHAGE_SIMD_AVX2) || defined(AUTOPHAGE_SIMD_AVX)
    #include <immintrin.h>
#elif defined(AUTOPHAGE_SIMD_SSE2)
    #include <emmintrin.h>
    #include <xmmintrin.h>
#elif defined(AUTOPHAGE_SIMD_NEON)
    #include <arm_neon.h>
#endif

namespace autophage::ecs {

// =============================================================================
// Velocity System - Integrates velocity into position
// =============================================================================

/// @brief Scalar implementation of velocity integration
class VelocitySystemScalar : public System<VelocitySystemScalar>
{
public:
    VelocitySystemScalar() : System("VelocitySystem::Scalar") {}

    void update(World& world, f32 dt) override
    {
        auto q = world.query<Transform, Velocity>();
        q.forEach([dt](Entity /*entity*/, Transform& transform, Velocity& velocity) {
            // Linear velocity integration: position += velocity * dt
            transform.position.x += velocity.linear.x * dt;
            transform.position.y += velocity.linear.y * dt;
            transform.position.z += velocity.linear.z * dt;
        });
    }
};

/// @brief SIMD implementation of velocity integration (processes 4 entities at a time)
class VelocitySystemSIMD : public System<VelocitySystemSIMD>
{
public:
    VelocitySystemSIMD() : System("VelocitySystem::SIMD") {}

    void update(World& world, f32 dt) override
    {
        // For now, fall back to scalar - full SoA implementation needed for true SIMD
        // This demonstrates the variant pattern; true SIMD requires SoA component storage

#if defined(AUTOPHAGE_SIMD_SSE2) || defined(AUTOPHAGE_SIMD_AVX2)
        // Single-entity SIMD (still faster due to vectorized operations)
        __m128 dtVec = _mm_set1_ps(dt);

        auto q = world.query<Transform, Velocity>();
        q.forEach([dtVec](Entity /*entity*/, Transform& transform, Velocity& velocity) {
            // Load position and velocity
            __m128 pos = _mm_loadu_ps(&transform.position.x);
            __m128 vel = _mm_loadu_ps(&velocity.linear.x);

            // pos += vel * dt
            __m128 delta = _mm_mul_ps(vel, dtVec);
            pos = _mm_add_ps(pos, delta);

            // Store result
            _mm_storeu_ps(&transform.position.x, pos);
        });
#else
        // Fallback to scalar
        auto q = world.query<Transform, Velocity>();
        q.forEach([dt](Entity /*entity*/, Transform& transform, Velocity& velocity) {
            transform.position.x += velocity.linear.x * dt;
            transform.position.y += velocity.linear.y * dt;
            transform.position.z += velocity.linear.z * dt;
        });
#endif
    }
};

/// @brief Velocity system with hot-swappable implementations
class VelocitySystem : public System<VelocitySystem>, public IVariantSystem
{
public:
    VelocitySystem() : System("VelocitySystem"), currentVariant_(SystemVariant::Scalar)
    {
// Check SIMD support at runtime and default to best available
#if defined(AUTOPHAGE_SIMD_AVX2) || defined(AUTOPHAGE_SIMD_SSE2)
        currentVariant_ = SystemVariant::SIMD;
#endif
    }

    void update(World& world, f32 dt) override
    {
        if (!enabled_)
            return;

        switch (currentVariant_) {
            case SystemVariant::SIMD:
                updateSIMD(world, dt);
                break;
            case SystemVariant::Scalar:
            case SystemVariant::Approximate:
            case SystemVariant::GPU:
            default:
                updateScalar(world, dt);
                break;
        }
    }

    [[nodiscard]] std::vector<SystemVariant> availableVariants() const override
    {
        std::vector<SystemVariant> variants = {SystemVariant::Scalar};
#if defined(AUTOPHAGE_SIMD_AVX2) || defined(AUTOPHAGE_SIMD_SSE2)
        variants.push_back(SystemVariant::SIMD);
#endif
        return variants;
    }

    [[nodiscard]] SystemVariant currentVariant() const noexcept override { return currentVariant_; }

    bool switchVariant(SystemVariant variant) override
    {
        auto available = availableVariants();
        for (auto v : available) {
            if (v == variant) {
                currentVariant_ = variant;
                return true;
            }
        }
        return false;
    }

private:
    void updateScalar(World& world, f32 dt)
    {
        auto q = world.query<Transform, Velocity>();
        q.forEach([dt](Entity /*entity*/, Transform& transform, Velocity& velocity) {
            transform.position.x += velocity.linear.x * dt;
            transform.position.y += velocity.linear.y * dt;
            transform.position.z += velocity.linear.z * dt;
        });
    }

    void updateSIMD(World& world, f32 dt)
    {
#if defined(AUTOPHAGE_SIMD_SSE2) || defined(AUTOPHAGE_SIMD_AVX2)
        __m128 dtVec = _mm_set1_ps(dt);

        auto q = world.query<Transform, Velocity>();
        q.forEach([dtVec](Entity /*entity*/, Transform& transform, Velocity& velocity) {
            __m128 pos = _mm_loadu_ps(&transform.position.x);
            __m128 vel = _mm_loadu_ps(&velocity.linear.x);
            pos = _mm_add_ps(pos, _mm_mul_ps(vel, dtVec));
            _mm_storeu_ps(&transform.position.x, pos);
        });
#else
        updateScalar(world, dt);
#endif
    }

    SystemVariant currentVariant_;
    bool enabled_ = true;
};

// =============================================================================
// Gravity System - Applies gravity to velocity
// =============================================================================

/// @brief Applies gravity to entities with Mass and Velocity
class GravitySystem : public System<GravitySystem>
{
public:
    GravitySystem() : System("GravitySystem") {}

    void update(World& world, f32 dt) override
    {
        // Global gravity for entities without custom Gravity component
        constexpr Vec3 defaultGravity{0, -9.81f, 0};

        auto q = world.query<Velocity, Mass>();
        q.forEach([&](Entity entity, Velocity& velocity, [[maybe_unused]] Mass& mass) {
            // Check for custom gravity
            const Gravity* customGravity = world.getComponent<Gravity>(entity);
            const Vec3& gravity = customGravity ? customGravity->value : defaultGravity;

            // Apply gravity: v += g * dt
            velocity.linear.x += gravity.x * dt;
            velocity.linear.y += gravity.y * dt;
            velocity.linear.z += gravity.z * dt;
        });
    }
};

// =============================================================================
// Acceleration System - Applies acceleration to velocity
// =============================================================================

/// @brief Applies acceleration to velocity
class AccelerationSystem : public System<AccelerationSystem>
{
public:
    AccelerationSystem() : System("AccelerationSystem") {}

    void update(World& world, f32 dt) override
    {
        auto q = world.query<Velocity, Acceleration>();
        q.forEach([dt](Entity /*entity*/, Velocity& velocity, Acceleration& accel) {
            velocity.linear.x += accel.value.x * dt;
            velocity.linear.y += accel.value.y * dt;
            velocity.linear.z += accel.value.z * dt;
        });
    }
};

// =============================================================================
// Transform Hierarchy System - Updates world transforms from hierarchy
// =============================================================================

/// @brief Updates transforms based on parent-child hierarchy
class HierarchySystem : public System<HierarchySystem>
{
public:
    HierarchySystem() : System("HierarchySystem") {}

    void update(World& world, [[maybe_unused]] f32 dt) override
    {
        // Process entities by depth level (root first, then children)
        // This is a simplified version - a full implementation would sort by depth

        auto q = world.query<Transform, Hierarchy>();
        q.forEach([&world](Entity entity, Transform& transform, Hierarchy& hierarchy) {
            if (hierarchy.hasParent()) {
                // Get parent's transform
                const Transform* parentTransform = world.getComponent<Transform>(hierarchy.parent);
                if (parentTransform) {
                    // Simple additive transform (rotation should use quaternion multiplication)
                    // This is a placeholder - real implementation needs proper matrix math
                    transform.position.x += parentTransform->position.x;
                    transform.position.y += parentTransform->position.y;
                    transform.position.z += parentTransform->position.z;
                }
            }
            (void)entity;  // Suppress unused warning
        });
    }
};

// =============================================================================
// Bounds Update System - Updates AABB from transform
// =============================================================================

/// @brief Updates AABB based on transform (simplified)
class BoundsSystem : public System<BoundsSystem>
{
public:
    BoundsSystem() : System("BoundsSystem") {}

    void update(World& world, [[maybe_unused]] f32 dt) override
    {
        auto q = world.query<Transform, AABB>();
        q.forEach([](Entity /*entity*/, Transform& transform, AABB& aabb) {
            // Simple offset - real implementation would transform all 8 corners
            Vec3 center = aabb.center();
            Vec3 extents = aabb.extents();

            // Translate bounds by position
            aabb.min = Vec3{transform.position.x + center.x - extents.x,
                            transform.position.y + center.y - extents.y,
                            transform.position.z + center.z - extents.z};
            aabb.max = Vec3{transform.position.x + center.x + extents.x,
                            transform.position.y + center.y + extents.y,
                            transform.position.z + center.z + extents.z};
        });
    }
};

// =============================================================================
// Cleanup System - Removes destroyed entities
// =============================================================================

/// @brief Removes entities marked with Destroyed tag
class CleanupSystem : public System<CleanupSystem>
{
public:
    CleanupSystem() : System("CleanupSystem") {}

    void update(World& world, [[maybe_unused]] f32 dt) override
    {
        // Collect entities to destroy (can't modify while iterating)
        std::vector<Entity> toDestroy;

        // Check for Destroyed tag
        auto& destroyedArray = world.componentRegistry().getArray<Destroyed>();
        destroyedArray.forEach([&toDestroy](Entity entity, [[maybe_unused]] Destroyed& /*tag*/) {
            toDestroy.push_back(entity);
        });

        // Destroy collected entities
        for (Entity entity : toDestroy) {
            world.destroyEntity(entity);
        }
    }
};

}  // namespace autophage::ecs
