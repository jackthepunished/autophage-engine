#pragma once

/// @file profiler.hpp
/// @brief Main profiler interface for Autophage Engine

#include <autophage/core/types.hpp>

#include <chrono>
#include <string>
#include <vector>

namespace autophage {

// =============================================================================
// Time Types
// =============================================================================

using Clock = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;
using Duration = std::chrono::nanoseconds;

// =============================================================================
// Frame Statistics
// =============================================================================

/// @brief Statistics for a single frame
struct FrameStats
{
    FrameNumber frameNumber = 0;
    Duration totalTime{0};
    Duration updateTime{0};
    Duration renderTime{0};
    u32 entityCount = 0;
    u32 systemCount = 0;
    usize memoryUsed = 0;

    // Hardware metrics (optional/platform dependent)
    u64 cpuCycles = 0;
    u64 cacheMisses = 0;
    u64 branchMispredictions = 0;
    u64 contextSwitches = 0;

    // Memory metrics
    u64 allocationCount = 0;
    u64 deallocationCount = 0;
};

/// @brief Aggregated statistics over multiple frames
struct ProfilerStats
{
    // Frame time statistics
    Duration avgFrameTime{0};
    Duration minFrameTime{Duration::max()};
    Duration maxFrameTime{0};
    Duration p95FrameTime{0};
    Duration p99FrameTime{0};

    // Frame rate
    f64 avgFps = 0.0;
    f64 minFps = 0.0;
    f64 maxFps = 0.0;

    // Spike detection
    u32 spikeCount = 0;
    Duration spikeThreshold{0};

    // Sample count
    u64 sampleCount = 0;

    // Aggregated Hardware metrics
    f64 avgCacheMisses = 0.0;
    f64 avgBranchMispredictions = 0.0;
};

// =============================================================================
// Profiler Zone
// =============================================================================

/// @brief Represents a profiled code section
struct ProfileZone
{
    u64 id = 0;
    const char* name = nullptr;
    const char* file = nullptr;
    u32 line = 0;
    Duration totalTime{0};
    Duration selfTime{0};
    u64 callCount = 0;
    u64 parentId = 0;
};

// =============================================================================
// Profiler Interface
// =============================================================================

/// @brief Initialize the profiler
/// @param historySize Number of frames to keep in history
void initProfiler(usize historySize = 300);

/// @brief Shutdown the profiler
void shutdownProfiler();

/// @brief Begin a new frame
void beginFrame();

/// @brief End the current frame
void endFrame();

/// @brief Get current frame number
[[nodiscard]] FrameNumber getCurrentFrame();

/// @brief Get statistics for current frame
[[nodiscard]] const FrameStats& getCurrentFrameStats();

/// @brief Get aggregated profiler statistics
[[nodiscard]] ProfilerStats getProfilerStats();

/// @brief Get frame history
[[nodiscard]] const std::vector<FrameStats>& getFrameHistory();

/// @brief Reset profiler statistics
void resetProfilerStats();

// =============================================================================
// Zone Management
// =============================================================================

/// @brief Begin a profiling zone
/// @param name Zone name (must be string literal)
/// @param file Source file
/// @param line Source line
/// @return Zone ID
[[nodiscard]] u64 beginZone(const char* name, const char* file = nullptr, u32 line = 0);

/// @brief End a profiling zone
/// @param zoneId Zone ID returned by beginZone
void endZone(u64 zoneId);

/// @brief Get all zones from current frame
[[nodiscard]] const std::vector<ProfileZone>& getZones();

// =============================================================================
// Metric Recording
// =============================================================================

/// @brief Record a counter value
void recordCounter(const char* name, i64 value);

/// @brief Record a gauge value
void recordGauge(const char* name, f64 value);

/// @brief Record a memory allocation
void recordAllocation(usize bytes, const char* tag = nullptr);

/// @brief Record a memory deallocation
void recordDeallocation(usize bytes, const char* tag = nullptr);

// =============================================================================
// Utility Macros
// =============================================================================

/// @brief Profile a scope with automatic begin/end
#define AUTOPHAGE_PROFILE_SCOPE(name) \
    ::autophage::ScopedTimer _profiler_scope_##__LINE__(name, __FILE__, __LINE__)

/// @brief Profile a function
#define AUTOPHAGE_PROFILE_FUNCTION() AUTOPHAGE_PROFILE_SCOPE(__func__)

}  // namespace autophage
