#include <autophage/core/assert.hpp>
#include <autophage/core/logger.hpp>
#include <autophage/window/window.hpp>

// Prevent SDL from redefining main
#define SDL_MAIN_HANDLED
#include <SDL.h>

namespace autophage {

class WindowSDL : public IWindow
{
public:
    ~WindowSDL() override
    {
        if (window_) {
            SDL_DestroyWindow(window_);
        }
        SDL_Quit();
    }

    bool init(const WindowConfig& config) override
    {
        // Initialize SDL Video subsystem
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            LOG_ERROR("Failed to initialize SDL: {}", SDL_GetError());
            return false;
        }

        // Configure window flags
        u32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
        if (config.fullscreen) {
            flags |= SDL_WINDOW_FULLSCREEN;
        }

        // Create the window
        window_ = SDL_CreateWindow(config.title.c_str(), SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED, static_cast<int>(config.width),
                                   static_cast<int>(config.height), flags);

        if (!window_) {
            LOG_ERROR("Failed to create SDL window: {}", SDL_GetError());
            return false;
        }

        width_ = config.width;
        height_ = config.height;
        LOG_INFO("Window created: {}x{}", width_, height_);

        return true;
    }

    void pollEvents() override
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    shouldClose_ = true;
                    break;
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        width_ = static_cast<u32>(event.window.data1);
                        height_ = static_cast<u32>(event.window.data2);
                    }
                    break;
                    // TODO: Input handling
            }
        }
    }

    [[nodiscard]] bool shouldClose() const override { return shouldClose_; }

    void swapBuffers() override
    {
        // Placeholder for swap buffers (requires GL/Vulkan context)
        // For now, if we had a surface, we would update it
        // SDL_UpdateWindowSurface(window_);
    }

    [[nodiscard]] u32 width() const override { return width_; }
    [[nodiscard]] u32 height() const override { return height_; }

    [[nodiscard]] void* nativeHandle() const override { return static_cast<void*>(window_); }

private:
    SDL_Window* window_ = nullptr;
    bool shouldClose_ = false;
    u32 width_ = 0;
    u32 height_ = 0;
};

std::unique_ptr<IWindow> createWindow()
{
    return std::make_unique<WindowSDL>();
}

}  // namespace autophage
