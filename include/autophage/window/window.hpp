#pragma once

/// @file window.hpp
/// @brief Windowing system abstraction

#include <autophage/core/types.hpp>

#include <memory>
#include <string>

namespace autophage {

/// @brief Configuration for creating a window
struct WindowConfig
{
    std::string title = "Autophage Engine";
    u32 width = 1280;
    u32 height = 720;
    bool fullscreen = false;
    bool vsync = true;
};

/// @brief Abstract window interface
class IWindow
{
public:
    virtual ~IWindow() = default;

    /// @brief Initialize the window
    /// @return Success or failure
    [[nodiscard]] virtual bool init(const WindowConfig& config) = 0;

    /// @brief Process pending events (input/window)
    virtual void pollEvents() = 0;

    /// @brief Check if the window should close
    [[nodiscard]] virtual bool shouldClose() const = 0;

    /// @brief Swap buffers (presents the rendered frame)
    virtual void present() = 0;

    /// @brief Clear the screen with a specific color
    virtual void clear(u8 r, u8 g, u8 b, u8 a = 255) = 0;

    /// @brief Draw a filled rectangle (debug/placeholder rendering)
    virtual void drawRect(i32 x, i32 y, i32 w, i32 h, u8 r, u8 g, u8 b, u8 a = 255) = 0;

    /// @brief Get window width
    [[nodiscard]] virtual u32 width() const = 0;

    /// @brief Get window height
    [[nodiscard]] virtual u32 height() const = 0;

    /// @brief Get native window handle (void* to avoid exposing SDL headers here)
    [[nodiscard]] virtual void* nativeHandle() const = 0;
};

/// @brief Create a platform-specific window
std::unique_ptr<IWindow> createWindow();

}  // namespace autophage
