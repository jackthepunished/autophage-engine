---
name: horizon-ui
description: Expert guide for Horizon Engine's UI system. Use when modifying the editor interface, creating debug overlays, or using ImGui.
---

# Horizon Engine UI Architecture

This skill provides guidelines for the user interface.

## 1. ImGui Integration (`engine/ui/imgui_layer.cpp`)

- **`ImGuiLayer`**: Manage the frame context.
- **Docking**: Enabled by default.
- **Backend**: Vulkan/OpenGL backends handled internally.

## 2. Creating Editor Panels

- implement `on_imgui_render()` in your layer or system.
- Use `ImGui::Begin("Panel Name")` / `ImGui::End()`.

## 3. UI Components (`scene_hierarchy_panel.cpp`)

- **Drawing Components**: Use `draw_component<T>()` helper.
- **Widgets**:
  - `ui::draw_vec3_control()`: Standard XYZ drag float.
  - `ui::draw_float_control()`: Standard float input.

## 4. Debug Overlays (`debug_overlay.hpp`)

- Use `DebugOverlay` for in-game stats (FPS, entity count).
- Lightweight, not interactive (usually).

## 5. Style & Theme

- The engine uses a custom dark theme.
- Do not manually change colors unless necessary (`ImGui::PushStyleColor`).
- Use the provided UI helpers in `engine/ui/ui.h` (assuming strictly UI helper header exists, otherwise general ImGui best practices).
