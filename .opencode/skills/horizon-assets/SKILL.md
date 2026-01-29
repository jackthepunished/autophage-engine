---
name: horizon-assets
description: Expert guide for Horizon Engine's Asset Management. Use when loading models, textures, or managing the asset registry.
---

# Horizon Engine Asset System

This skill provides guidelines for asset management.

## 1. Asset Registry (`engine/assets/asset_registry.hpp`)

- **Role**: Global store of all loaded assets.
- **`AssetHandle`**: Use this UUID-based handle to reference assets, never raw pointers.
  - Handles are safe to serialize.

## 2. Supported Asset Types

- **Texture2D**: loaded via `Texture::create(path)`.
- **Mesh/Model**: loaded via `Model::create(path)`.
- **Cubemap**: for skyboxes.
- **Material**: usage of shaders and textures combined.

## 3. Asynchronous Loading

- Assets are loaded on a background thread where possible.
- Check `Asset::is_loaded()` before using if requiring immediate access.

## 4. Usage in Components

- Store `AssetHandle` in components (e.g. `MeshComponent`).
- The `RenderSystem` resolves these handles to underlying GPU resources at draw time.

```cpp
struct MeshComponent {
    AssetHandle mesh_handle;
    AssetHandle material_handle;
};
```
