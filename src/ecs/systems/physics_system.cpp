#include <autophage/ecs/systems/physics_system.hpp>
#include <autophage/ecs/world.hpp>

#include <immintrin.h>  // AVX/SSE

#pragma warning(disable : 4250)  // 'inherits via dominance'

namespace autophage::ecs {

PhysicsSystem::PhysicsSystem() : System("PhysicsSystem") {}

void PhysicsSystem::update(World& world, f32 dt)
{
    if (currentVariant_ == SystemVariant::SIMD) {
        updateSIMD(world, dt);
    } else {
        updateScalar(world, dt);
    }
}

std::vector<SystemVariant> PhysicsSystem::availableVariants() const
{
    return {SystemVariant::Scalar, SystemVariant::SIMD};
}

SystemVariant PhysicsSystem::currentVariant() const noexcept
{
    return currentVariant_;
}

bool PhysicsSystem::switchVariant(SystemVariant variant)
{
    if (variant == SystemVariant::Scalar || variant == SystemVariant::SIMD) {
        currentVariant_ = variant;
        return true;
    }
    return false;
}

void PhysicsSystem::updateScalar(World& world, f32 dt)
{
    for (auto [entity, transform, velocity] : world.view<Transform, Velocity>()) {
        // Apply velocity to position
        transform.position += velocity.linear * dt;

        // Apply generic gravity/acceleration if present
        // (Simplified for demo)
    }
}

void PhysicsSystem::updateSIMD(World& world, f32 dt)
{
    // Single-Entity SIMD (processing per-vector with SIMD instructions)
    // Because storage is AoS (Vec3 structs), we load one Vec3 at a time.

    __m128 dt_vec = _mm_set1_ps(dt);

    for (auto [entity, transform, velocity] : world.view<Transform, Velocity>()) {
        // Load position and velocity
        // Ensure alignment? Vec3 is aligned to 16 bytes.
        // Assuming ComponentArray allocation is aligned properly.
        // If not aligned, use _mm_loadu_ps.
        // Our Allocator ensures alignment? Components are in `std::vector` or raw chunk?
        // `ComponentArray` uses `PagedArray`?
        // For safety, store pointer to Vec3.

        f32* pos_ptr = &transform.position.x;
        f32* vel_ptr = &velocity.linear.x;

        // Check alignment
        if ((reinterpret_cast<uintptr_t>(pos_ptr) & 15) == 0 &&
            (reinterpret_cast<uintptr_t>(vel_ptr) & 15) == 0) {
            __m128 pos = _mm_load_ps(pos_ptr);
            __m128 vel = _mm_load_ps(vel_ptr);

            // pos += vel * dt
            __m128 result = _mm_add_ps(pos, _mm_mul_ps(vel, dt_vec));

            _mm_store_ps(pos_ptr, result);
        } else {
            // Fallback unaligned
            __m128 pos = _mm_loadu_ps(pos_ptr);
            __m128 vel = _mm_loadu_ps(vel_ptr);

            __m128 result = _mm_add_ps(pos, _mm_mul_ps(vel, dt_vec));

            _mm_storeu_ps(pos_ptr, result);
        }
    }
}

}  // namespace autophage::ecs
