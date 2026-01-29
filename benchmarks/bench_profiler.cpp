/// @file bench_profiler.cpp
/// @brief Profiler overhead benchmarks

#include <nanobench.h>

#include <autophage/profiler/profiler.hpp>
#include <autophage/profiler/scoped_timer.hpp>

using namespace autophage;

int main() {
    initProfiler(1000);

    ankerl::nanobench::Bench bench;
    bench.title("Profiler Overhead Benchmarks");
    bench.warmup(100);
    bench.relative(true);

    // Baseline: empty function
    bench.run("Baseline (empty)", [&] {
        ankerl::nanobench::doNotOptimizeAway(0);
    });

    // Clock::now() overhead
    bench.run("Clock::now()", [&] {
        auto t = Clock::now();
        ankerl::nanobench::doNotOptimizeAway(t);
    });

    // beginFrame/endFrame overhead
    bench.run("beginFrame/endFrame", [&] {
        beginFrame();
        endFrame();
    });

    // Zone creation overhead
    beginFrame();
    bench.run("beginZone/endZone", [&] {
        u64 id = beginZone("BenchZone");
        endZone(id);
    });
    endFrame();

    // ScopedTimer overhead
    beginFrame();
    bench.run("ScopedTimer", [&] {
        ScopedTimer timer("BenchScope");
        ankerl::nanobench::doNotOptimizeAway(&timer);
    });
    endFrame();

    // ManualTimer overhead
    bench.run("ManualTimer start/stop", [&] {
        ManualTimer timer;
        timer.start();
        timer.stop();
        ankerl::nanobench::doNotOptimizeAway(timer.elapsed());
    });

    // Nested zones
    beginFrame();
    bench.run("Nested zones (3 levels)", [&] {
        u64 id1 = beginZone("Level1");
        {
            u64 id2 = beginZone("Level2");
            {
                u64 id3 = beginZone("Level3");
                endZone(id3);
            }
            endZone(id2);
        }
        endZone(id1);
    });
    endFrame();

    // getProfilerStats overhead
    for (int i = 0; i < 100; ++i) {
        beginFrame();
        endFrame();
    }

    bench.run("getProfilerStats()", [&] {
        auto stats = getProfilerStats();
        ankerl::nanobench::doNotOptimizeAway(stats.avgFrameTime);
    });

    shutdownProfiler();

    return 0;
}
