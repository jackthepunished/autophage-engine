/// @file test_profiler.cpp
/// @brief Tests for profiler system

#include <catch2/catch_test_macros.hpp>
#include <autophage/profiler/profiler.hpp>

#include <thread>
#include <chrono>

using namespace autophage;

TEST_CASE("Profiler initialization", "[profiler]") {
    initProfiler(100);

    SECTION("Frame counting works") {
        REQUIRE(getCurrentFrame() == 0);

        beginFrame();
        endFrame();
        REQUIRE(getCurrentFrame() == 1);

        beginFrame();
        endFrame();
        REQUIRE(getCurrentFrame() == 2);
    }

    SECTION("Frame history is recorded") {
        for (int i = 0; i < 10; ++i) {
            beginFrame();
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            endFrame();
        }

        const auto& history = getFrameHistory();
        REQUIRE(history.size() == 10);
    }

    resetProfilerStats();
    shutdownProfiler();
}

TEST_CASE("Profiler statistics", "[profiler]") {
    initProfiler(100);

    // Run some frames
    for (int i = 0; i < 50; ++i) {
        beginFrame();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        endFrame();
    }

    auto stats = getProfilerStats();

    SECTION("Sample count is correct") {
        REQUIRE(stats.sampleCount == 50);
    }

    SECTION("Frame times are positive") {
        REQUIRE(stats.avgFrameTime.count() > 0);
        REQUIRE(stats.minFrameTime.count() > 0);
        REQUIRE(stats.maxFrameTime.count() > 0);
    }

    SECTION("Min <= Avg <= Max") {
        REQUIRE(stats.minFrameTime <= stats.avgFrameTime);
        REQUIRE(stats.avgFrameTime <= stats.maxFrameTime);
    }

    SECTION("FPS is calculated") {
        REQUIRE(stats.avgFps > 0);
    }

    shutdownProfiler();
}

TEST_CASE("Profile zones", "[profiler]") {
    initProfiler(100);

    beginFrame();

    SECTION("Zone timing") {
        u64 zoneId = beginZone("TestZone", __FILE__, __LINE__);
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        endZone(zoneId);

        const auto& zones = getZones();
        REQUIRE(zones.size() == 1);
        REQUIRE(zones[0].name == std::string("TestZone"));
        REQUIRE(zones[0].totalTime.count() > 0);
    }

    endFrame();
    shutdownProfiler();
}
