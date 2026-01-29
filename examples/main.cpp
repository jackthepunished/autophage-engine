/// @file main.cpp
/// @brief Autophage Engine Demo Application

#include <autophage/core/types.hpp>
#include <autophage/core/platform.hpp>
#include <autophage/core/logger.hpp>
#include <autophage/core/memory.hpp>
#include <autophage/profiler/profiler.hpp>
#include <autophage/profiler/scoped_timer.hpp>

#include <iostream>
#include <thread>
#include <chrono>
#include <random>

using namespace autophage;

// Simulated workload
void simulatePhysics(f64 dt) {
    AUTOPHAGE_PROFILE_FUNCTION();
    
    // Simulate some work
    volatile f64 result = 0;
    for (int i = 0; i < 1000; ++i) {
        result += std::sin(static_cast<f64>(i) * dt);
    }
}

void simulateRendering() {
    AUTOPHAGE_PROFILE_FUNCTION();
    
    // Simulate rendering work
    std::this_thread::sleep_for(std::chrono::microseconds(500));
}

void runFrame(f64 dt) {
    AUTOPHAGE_PROFILE_SCOPE("Frame");
    
    simulatePhysics(dt);
    simulateRendering();
}

int main() {
    // Initialize systems
    initLogger("autophage-demo", LogLevel::Debug);
    initProfiler(300);  // Keep 300 frames of history (5 seconds at 60fps)

    LOG_INFO("===========================================");
    LOG_INFO("  Autophage Engine Demo");
    LOG_INFO("===========================================");

    // Print platform info
    auto platform = getPlatformInfo();
    LOG_INFO("Platform: {}", platform.name);
    LOG_INFO("Compiler: {} (version {})", platform.compiler, platform.compilerVersion);
    LOG_INFO("Arch: {}", platform.arch);
    LOG_INFO("SIMD Level: {} bits", platform.simdLevel);
    LOG_INFO("Build: {}", platform.build);
    LOG_INFO("");

    // Test memory allocators
    {
        LOG_INFO("Testing memory allocators...");
        
        LinearAllocator arena(1024 * 1024, MemoryTag::Temporary);
        LOG_INFO("  LinearAllocator: {} bytes capacity", arena.capacity());

        // Allocate some data
        auto* data = arena.allocArray<f32>(1000);
        LOG_INFO("  Allocated 1000 floats, used: {} bytes", arena.used());

        arena.reset();
        LOG_INFO("  After reset: {} bytes used", arena.used());
    }

    // Run simulation loop
    LOG_INFO("");
    LOG_INFO("Running simulation (100 frames)...");

    constexpr int FRAME_COUNT = 100;
    constexpr f64 TARGET_DT = 1.0 / 60.0;  // 60 FPS target

    for (int frame = 0; frame < FRAME_COUNT; ++frame) {
        beginFrame();
        
        runFrame(TARGET_DT);
        
        endFrame();

        // Print progress every 25 frames
        if ((frame + 1) % 25 == 0) {
            auto stats = getProfilerStats();
            LOG_INFO("  Frame {}: avg={:.2f}ms, fps={:.1f}",
                     frame + 1,
                     std::chrono::duration<f64, std::milli>(stats.avgFrameTime).count(),
                     stats.avgFps);
        }
    }

    // Print final statistics
    LOG_INFO("");
    LOG_INFO("=== Final Statistics ===");
    
    auto stats = getProfilerStats();
    LOG_INFO("Frames: {}", stats.sampleCount);
    LOG_INFO("Avg Frame Time: {:.3f} ms", 
             std::chrono::duration<f64, std::milli>(stats.avgFrameTime).count());
    LOG_INFO("Min Frame Time: {:.3f} ms",
             std::chrono::duration<f64, std::milli>(stats.minFrameTime).count());
    LOG_INFO("Max Frame Time: {:.3f} ms",
             std::chrono::duration<f64, std::milli>(stats.maxFrameTime).count());
    LOG_INFO("P95 Frame Time: {:.3f} ms",
             std::chrono::duration<f64, std::milli>(stats.p95FrameTime).count());
    LOG_INFO("P99 Frame Time: {:.3f} ms",
             std::chrono::duration<f64, std::milli>(stats.p99FrameTime).count());
    LOG_INFO("");
    LOG_INFO("Avg FPS: {:.1f}", stats.avgFps);
    LOG_INFO("Min FPS: {:.1f}", stats.minFps);
    LOG_INFO("Max FPS: {:.1f}", stats.maxFps);
    LOG_INFO("");
    LOG_INFO("Spikes (>{:.1f}ms): {}", 
             std::chrono::duration<f64, std::milli>(stats.spikeThreshold).count(),
             stats.spikeCount);

    // Memory statistics
    LOG_INFO("");
    LOG_INFO("=== Memory Statistics ===");
    auto memStats = getTotalMemoryStats();
    LOG_INFO("Current: {} bytes", memStats.currentBytes);
    LOG_INFO("Peak: {} bytes", memStats.peakBytes);
    LOG_INFO("Total Allocations: {}", memStats.totalAllocations);
    LOG_INFO("Total Deallocations: {}", memStats.totalDeallocations);

    // Cleanup
    LOG_INFO("");
    LOG_INFO("Shutting down...");
    shutdownProfiler();
    shutdownLogger();

    return 0;
}
