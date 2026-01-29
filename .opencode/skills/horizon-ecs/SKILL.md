---
name: horizon-ecs
description: Expert guide for Horizon Engine's Entity Component System. Use when creating new components, writing systems, or managing the scene graph.
---

# Horizon Engine ECS Architecture

This skill provides guidelines for working with the Entity Component System.

## 1. Components (`engine/scene/components.hpp`)

**Rule**: Components must be POD (Plain Old Data) structs or aggregates.

- **Storage**: Components are stored in contiguous arrays (like Enteer/EnTT).
- **No Logic**: Do not add methods to components (helpers are okay, but avoid complex logic).
- **Serialization**: Every component must be registered in `scene_serializer.cpp`.
- **Inspector**: Every component must have a UI draw function in `scene_hierarchy_panel.cpp`.

```cpp
// Example Component
struct GravityComponent {
    float mass = 1.0f;
    glm::vec3 direction = {0.0f, -9.8f, 0.0f};
};
```

## 2. Systems

**Rule**: Logic belongs in Systems (or Managers/Controllers).

- **Update Loops**: Systems usually run in `on_update(float ts)`.
- **Queries**: Iterate over view of entities with specific components.

```cpp
void PhysicsSystem::on_update(float ts) {
    auto view = m_registry.view<TransformComponent, RigidbodyComponent>();
    for (auto entity : view) {
        auto [transform, rb] = view.get<TransformComponent, RigidbodyComponent>(entity);
        // ... physics logic ...
    }
}
```

## 3. Scene Graph

- **Entities**: Are just IDs (`entt::entity`).
- **Parent/Child**: Handled by `RelationshipComponent` (if implemented) or manually transform hierarchy.
- **Scene**: Owns the registry and all entities.

## 4. Creating a New Component Checklist

1. **Define struct** in `components.hpp`.
2. **Register** in `Scene::copy_component` (if copyable).
3. **Serialize** in `SceneSerializer::serialize_entity`.
4. **Deserialize** in `SceneSerializer::deserialize_entity`.
5. **UI** in `SceneHierarchyPanel::draw_components`.
