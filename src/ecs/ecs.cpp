/// @file ecs.cpp
/// @brief ECS module implementation (non-template code)

#include <autophage/ecs/world.hpp>
#include <autophage/ecs/components.hpp>
#include <autophage/ecs/systems.hpp>

namespace autophage::ecs {

// Most ECS code is header-only templates.
// This file exists for:
// 1. Explicit template instantiations (if needed)
// 2. Non-template helper functions
// 3. Ensuring the library has at least one compilation unit

// Explicit instantiations for common component types (optional, for faster compile times)
// template class ComponentArray<Transform>;
// template class ComponentArray<Velocity>;
// template class ComponentArray<Hierarchy>;

}  // namespace autophage::ecs
