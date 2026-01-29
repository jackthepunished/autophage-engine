---
name: horizon-renderer
description: Expert guide for Horizon Engine's Renderer (Vulkan/OpenGL). Use when writing shaders, modifying the render graph, or touching RHI code.
---

# Horizon Engine Renderer Architecture

This skill provides guidelines for graphics programming within the engine.

## 1. RHI Abstraction (Render Hardware Interface)

We support multiple backends (Vulkan, OpenGL).

- **Rule**: Never call raw API functions (e.g., `glDrawArrays`, `vkCmdDraw`) in gameplay code.
- **Use**: `Renderer::submit()`, `RenderCommand::draw_indexed()`.
- **Buffers**: Use `VertexBuffer::create()`, `IndexBuffer::create()` factory functions.

## 2. Shaders (`shader.rules.yaml`)

**Rule**: Shaders are compiled to SPIR-V (for Vulkan) and cross-compiled for OpenGL.

- **Files**: `.glsl` usually containing both vertex and fragment stages (via `#ifdef` or separate files).
- **Layouts**: Use strict binding slots.
  - Set 0: Global (Camera, Scene)
  - Set 1: Material (Textures, Properties)
  - Set 2: Object (Transform)

## 3. Render Graph / Pass Management

- **Deferred Rendering**: We use a deferred pipeline (G-Buffer pass -> Lighting pass).
- **G-Buffer**: Contains Albedo, Normal, Roughness/Metalness/AO.
- **Post-Processing**: Runs after lighting (Bloom, Tonemapping).

## 4. Vulkan Specifics (`vulkan.rules.yaml`)

- **Validation**: Validation errors are fatal.
- **Barriers**: Must be explicit.
- **Descriptors**: Managed by `ImGui` or internal descriptor writers. Do not manually update descriptor sets unless working on core RHI.

## 5. Debugging

- **RenderDoc**: Use it to inspect frames.
- **Markers**: Use `Renderer::begin_debug_marker("Name")` / `end_debug_marker()` to label passes in RenderDoc.
