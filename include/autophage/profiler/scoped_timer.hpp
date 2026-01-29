#pragma once

/// @file scoped_timer.hpp
/// @brief RAII-based scoped timer for profiling

#include <autophage/profiler/profiler.hpp>

namespace autophage {

/// @brief RAII timer that automatically records zone duration
class ScopedTimer {
public:
    /// @brief Start timing a zone
    /// @param name Zone name (should be string literal)
    /// @param file Source file
    /// @param line Source line
    explicit ScopedTimer(const char* name, const char* file = nullptr, u32 line = 0)
        : zoneId_(beginZone(name, file, line)) {}

    /// @brief Stop timing and record duration
    ~ScopedTimer() {
        endZone(zoneId_);
    }

    // Non-copyable, non-moveable
    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;
    ScopedTimer(ScopedTimer&&) = delete;
    ScopedTimer& operator=(ScopedTimer&&) = delete;

private:
    u64 zoneId_;
};

/// @brief Manual timer for explicit start/stop
class ManualTimer {
public:
    ManualTimer() = default;

    /// @brief Start the timer
    void start() noexcept {
        startTime_ = Clock::now();
        running_ = true;
    }

    /// @brief Stop the timer
    void stop() noexcept {
        if (running_) {
            endTime_ = Clock::now();
            running_ = false;
        }
    }

    /// @brief Reset the timer
    void reset() noexcept {
        startTime_ = TimePoint{};
        endTime_ = TimePoint{};
        running_ = false;
    }

    /// @brief Get elapsed time
    [[nodiscard]] Duration elapsed() const noexcept {
        if (running_) {
            return std::chrono::duration_cast<Duration>(Clock::now() - startTime_);
        }
        return std::chrono::duration_cast<Duration>(endTime_ - startTime_);
    }

    /// @brief Get elapsed time in seconds
    [[nodiscard]] f64 elapsedSeconds() const noexcept {
        return std::chrono::duration<f64>(elapsed()).count();
    }

    /// @brief Get elapsed time in milliseconds
    [[nodiscard]] f64 elapsedMilliseconds() const noexcept {
        return std::chrono::duration<f64, std::milli>(elapsed()).count();
    }

    /// @brief Get elapsed time in microseconds
    [[nodiscard]] f64 elapsedMicroseconds() const noexcept {
        return std::chrono::duration<f64, std::micro>(elapsed()).count();
    }

    /// @brief Check if timer is running
    [[nodiscard]] bool isRunning() const noexcept { return running_; }

private:
    TimePoint startTime_{};
    TimePoint endTime_{};
    bool running_ = false;
};

}  // namespace autophage
