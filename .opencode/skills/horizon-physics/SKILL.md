---
name: horizon-physics
description: Expert guide for Horizon Engine's Physics system. Use when implementing character controllers, projectiles, interactions, or modifying the physics world.
---

# Horizon Engine Physics Architecture

This skill provides guidelines for the physics subsystem.

## 1. Core Physics Classes (`engine/physics/`)

- **`PhysicsWorld`**: The central simulation hub.
  - Manages the Jolt/PhysX scene (depending on backend).
  - Handles raycasts and shape queries.
- **`PhysicsInteractions`**: Handles collision callbacks.
  - Use `on_trigger_enter` / `on_trigger_exit` for game logic.

## 2. Character Controllers

- **`FPSCharacterController`**: Standard first-person controller.
  - Handles movement, gravity, and ground checking.
  - Use `move(vec3 velocity)` to apply motion.

## 3. Combat Systems

- **`ProjectileSystem`**: Manages bullet physics.
  - Raycast-based high velocity projectiles.
  - Physical objects for grenades/slow projectiles.
- **`HitboxSystem`**: Per-limb collision detection for entities.

## 4. Configuration

- **`PhysicsConfig`**: Global settings (Gravity, Steps per second).
- **Layers**:
  - `STATIC`: World geometry.
  - `DYNAMIC`: Moving props.
  - `PLAYER`: The local player.
  - `TRIGGER`: Non-physical usage zones.

## 5. Best Practices

- **Fixed Update**: All physics forces should be applied in `on_fixed_update()`, NOT `on_update()`.
- **Querying**: Use `PhysicsWorld::raycast()` for hitscan weapons or interaction checks.
