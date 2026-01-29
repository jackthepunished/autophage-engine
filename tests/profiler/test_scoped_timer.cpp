/// @file test_scoped_timer.cpp
/// @brief Tests for scoped timer utilities

#include <catch2/catch_test_macros.hpp>
#include <autophage/profiler/scoped_timer.hpp>
#include <autophage/profiler/profiler.hpp>

#include <thread>
#include <chrono>

using namespace autophage;

TEST_CASE("ManualTimer", "[profiler][timer]") {
    ManualTimer timer;

    SECTION("Initial state") {
        REQUIRE_FALSE(timer.isRunning());
        REQUIRE(timer.elapsed().count() == 0);
    }

    SECTION("Start and stop") {
        timer.start();
        REQUIRE(timer.isRunning());

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        timer.stop();
        REQUIRE_FALSE(timer.isRunning());
        REQUIRE(timer.elapsedMilliseconds() >= 9.0);  // Allow some tolerance
    }

    SECTION("Reset clears elapsed time") {
        timer.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        timer.stop();

        REQUIRE(timer.elapsedMilliseconds() > 0);

        timer.reset();
        REQUIRE(timer.elapsed().count() == 0);
        REQUIRE_FALSE(timer.isRunning());
    }

    SECTION("Elapsed while running") {
        timer.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        // Should return current elapsed time even while running
        double elapsed1 = timer.elapsedMilliseconds();
        REQUIRE(elapsed1 > 0);

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        double elapsed2 = timer.elapsedMilliseconds();
        REQUIRE(elapsed2 > elapsed1);
    }

    SECTION("Time unit conversions") {
        timer.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        timer.stop();

        double seconds = timer.elapsedSeconds();
        double milliseconds = timer.elapsedMilliseconds();
        double microseconds = timer.elapsedMicroseconds();

        REQUIRE(seconds >= 0.009);
        REQUIRE(milliseconds >= 9.0);
        REQUIRE(microseconds >= 9000.0);
    }
}

TEST_CASE("ScopedTimer with profiler", "[profiler][timer]") {
    initProfiler(100);
    beginFrame();

    SECTION("Automatic zone creation") {
        {
            ScopedTimer timer("ScopedTest", __FILE__, __LINE__);
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }

        const auto& zones = getZones();
        REQUIRE(zones.size() == 1);
        REQUIRE(zones[0].totalTime.count() > 0);
    }

    endFrame();
    shutdownProfiler();
}
