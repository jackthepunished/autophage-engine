/// @file profiler.cpp
/// @brief Profiler implementation

#include <autophage/profiler/profiler.hpp>
#include <autophage/core/logger.hpp>

#include <algorithm>
#include <atomic>
#include <mutex>
#include <numeric>

namespace autophage {

namespace {

// =============================================================================
// Profiler State
// =============================================================================

struct ProfilerState {
    std::vector<FrameStats> frameHistory;
    std::vector<ProfileZone> currentZones;
    std::vector<TimePoint> zoneStartTimes;

    FrameStats currentFrame;
    TimePoint frameStart;

    std::atomic<FrameNumber> frameNumber{0};
    std::atomic<bool> initialized{false};

    usize historySize = 300;
    std::mutex mutex;
};

ProfilerState g_profiler;

}  // namespace

// =============================================================================
// Initialization
// =============================================================================

void initProfiler(usize historySize) {
    std::lock_guard lock(g_profiler.mutex);

    g_profiler.historySize = historySize;
    g_profiler.frameHistory.reserve(historySize);
    g_profiler.currentZones.reserve(256);
    g_profiler.zoneStartTimes.reserve(256);
    g_profiler.frameNumber.store(0, std::memory_order_relaxed);
    g_profiler.initialized.store(true, std::memory_order_release);

    LOG_INFO("Profiler initialized with history size: {}", historySize);
}

void shutdownProfiler() {
    std::lock_guard lock(g_profiler.mutex);

    g_profiler.frameHistory.clear();
    g_profiler.currentZones.clear();
    g_profiler.zoneStartTimes.clear();
    g_profiler.initialized.store(false, std::memory_order_release);

    LOG_INFO("Profiler shut down");
}

// =============================================================================
// Frame Management
// =============================================================================

void beginFrame() {
    if (!g_profiler.initialized.load(std::memory_order_acquire)) {
        return;
    }

    g_profiler.frameStart = Clock::now();
    g_profiler.currentFrame = FrameStats{};
    g_profiler.currentFrame.frameNumber = g_profiler.frameNumber.load(std::memory_order_relaxed);
    g_profiler.currentZones.clear();
    g_profiler.zoneStartTimes.clear();
}

void endFrame() {
    if (!g_profiler.initialized.load(std::memory_order_acquire)) {
        return;
    }

    auto frameEnd = Clock::now();
    g_profiler.currentFrame.totalTime =
        std::chrono::duration_cast<Duration>(frameEnd - g_profiler.frameStart);

    // Add to history
    {
        std::lock_guard lock(g_profiler.mutex);

        if (g_profiler.frameHistory.size() >= g_profiler.historySize) {
            // Remove oldest frame
            g_profiler.frameHistory.erase(g_profiler.frameHistory.begin());
        }
        g_profiler.frameHistory.push_back(g_profiler.currentFrame);
    }

    g_profiler.frameNumber.fetch_add(1, std::memory_order_relaxed);
}

FrameNumber getCurrentFrame() {
    return g_profiler.frameNumber.load(std::memory_order_relaxed);
}

const FrameStats& getCurrentFrameStats() {
    return g_profiler.currentFrame;
}

const std::vector<FrameStats>& getFrameHistory() {
    return g_profiler.frameHistory;
}

// =============================================================================
// Statistics
// =============================================================================

ProfilerStats getProfilerStats() {
    std::lock_guard lock(g_profiler.mutex);

    ProfilerStats stats{};
    const auto& history = g_profiler.frameHistory;

    if (history.empty()) {
        return stats;
    }

    stats.sampleCount = history.size();

    // Calculate min, max, avg
    Duration total{0};
    for (const auto& frame : history) {
        total += frame.totalTime;
        if (frame.totalTime < stats.minFrameTime) {
            stats.minFrameTime = frame.totalTime;
        }
        if (frame.totalTime > stats.maxFrameTime) {
            stats.maxFrameTime = frame.totalTime;
        }
    }
    stats.avgFrameTime = total / static_cast<i64>(history.size());

    // Calculate percentiles
    std::vector<Duration> sortedTimes;
    sortedTimes.reserve(history.size());
    for (const auto& frame : history) {
        sortedTimes.push_back(frame.totalTime);
    }
    std::sort(sortedTimes.begin(), sortedTimes.end());

    auto getPercentile = [&](double p) -> Duration {
        usize idx = static_cast<usize>(p * static_cast<double>(sortedTimes.size() - 1));
        return sortedTimes[idx];
    };

    stats.p95FrameTime = getPercentile(0.95);
    stats.p99FrameTime = getPercentile(0.99);

    // Calculate FPS
    auto toSeconds = [](Duration d) {
        return std::chrono::duration<f64>(d).count();
    };

    stats.avgFps = 1.0 / toSeconds(stats.avgFrameTime);
    stats.minFps = 1.0 / toSeconds(stats.maxFrameTime);
    stats.maxFps = 1.0 / toSeconds(stats.minFrameTime);

    // Spike detection (frames > 2x average)
    stats.spikeThreshold = stats.avgFrameTime * 2;
    for (const auto& frame : history) {
        if (frame.totalTime > stats.spikeThreshold) {
            ++stats.spikeCount;
        }
    }

    return stats;
}

void resetProfilerStats() {
    std::lock_guard lock(g_profiler.mutex);
    g_profiler.frameHistory.clear();
}

// =============================================================================
// Zone Management
// =============================================================================

u64 beginZone(const char* name, const char* file, u32 line) {
    if (!g_profiler.initialized.load(std::memory_order_acquire)) {
        return 0;
    }

    u64 zoneId = g_profiler.currentZones.size();

    ProfileZone zone{};
    zone.id = zoneId;
    zone.name = name;
    zone.file = file;
    zone.line = line;
    zone.callCount = 1;

    g_profiler.currentZones.push_back(zone);
    g_profiler.zoneStartTimes.push_back(Clock::now());

    return zoneId;
}

void endZone(u64 zoneId) {
    if (!g_profiler.initialized.load(std::memory_order_acquire)) {
        return;
    }

    if (zoneId >= g_profiler.currentZones.size()) {
        return;
    }

    auto endTime = Clock::now();
    auto& zone = g_profiler.currentZones[zoneId];
    auto startTime = g_profiler.zoneStartTimes[zoneId];

    zone.totalTime = std::chrono::duration_cast<Duration>(endTime - startTime);
    zone.selfTime = zone.totalTime;  // TODO: Subtract child zones
}

const std::vector<ProfileZone>& getZones() {
    return g_profiler.currentZones;
}

// =============================================================================
// Metric Recording
// =============================================================================

void recordCounter(const char* /*name*/, i64 /*value*/) {
    // TODO: Implement counter tracking
}

void recordGauge(const char* /*name*/, f64 /*value*/) {
    // TODO: Implement gauge tracking
}

void recordAllocation(usize bytes, const char* /*tag*/) {
    g_profiler.currentFrame.memoryUsed += bytes;
}

void recordDeallocation(usize bytes, const char* /*tag*/) {
    if (g_profiler.currentFrame.memoryUsed >= bytes) {
        g_profiler.currentFrame.memoryUsed -= bytes;
    }
}

}  // namespace autophage
