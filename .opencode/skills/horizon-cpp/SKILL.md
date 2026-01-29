---
name: horizon-cpp
description: Expert guide for C++ development in Horizon Engine. Use when writing, reviewing, or refactoring C++ code to ensure compliance with project standards (C++20, ownership, naming, memory management).
---

# Horizon Engine C++ Standards

This skill enforces the project's strict C++ standards to maintain performance and safety.

## 1. Ownership & Memory (`cpp-core.rules.yaml`)

**Rule**: Never use `new` or `delete` manually.
**Reason**: Manual memory management is error-prone and leads to leaks.

- **Use**: `std::unique_ptr` for exclusive ownership.
- **Use**: `std::shared_ptr` for shared ownership (sparse use).
- **Use**: Reference (`&`) or `std::weak_ptr` for non-owning access.
- **Exceptions**: Inside custom `std::pmr::memory_resource` or low-level allocators.

## 2. Naming Conventions

Follow `docs/contributing/style_guide.md` strictly:

| Type | Case | Example |
|------|------|---------|
| **Types/Classes** | `PascalCase` | `MeshComponent`, `RenderSystem` |
| **Variables** | `snake_case` | `vertex_buffer`, `entity_id` |
| **Functions** | `snake_case` | `create_entity()`, `render_frame()` |
| **Private Members** | `m_snake_case` | `m_width`, `m_render_context` |
| **Macros** | `UPPER_SNAKE_CASE` | `HZ_ASSERT`, `HZ_LOG` |
| **Namespaces** | `lower_case` | `hz`, `hz::renderer` |

## 3. Modern C++ Features (C++20)

- **Concepts**: Use `requires` for template constraints instead of SFINAE.
- **Ranges**: Use `std::ranges` for algorithms where possible.
- **Modules**: (If enabled) Use standard headers, avoid pollution.
- **Spans**: Use `std::spanLike` or `std::span` for array views instead of pointer + size.

## 4. Safety & Performance

- **`[[nodiscard]]`**: Use for all factory functions or functions where return value matters.
- **`noexcept`**: Mark move constructors and destructors as `noexcept` to allow vector optimizations.
- **Asserts**: Use `HZ_ASSERT` liberally to document invariants.
- **Casts**: Avoid `c-style` casts. Use `static_cast`, `dynamic_cast`, or `reinterpret_cast` (carefully).

## 5. File Structure

- **Headers**: `#pragma once` at the top.
- **Include Order**:
  1. "hhp" (Corresponding header)
  2. <Standard Library>
  3. <Third Party>
  4. "Engine/..." (Project headers)

## Reference Files
- `docs/contributing/style_guide.md`
- `.agent/rules/cpp-core.rules.yaml`
