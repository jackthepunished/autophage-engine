/// @file bench_memory.cpp
/// @brief Memory allocation benchmarks

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <autophage/core/memory.hpp>

#include <vector>
#include <memory>

using namespace autophage;

int main() {
    ankerl::nanobench::Bench bench;
    bench.title("Memory Allocation Benchmarks");
    bench.warmup(100);
    bench.relative(true);

    // Baseline: std::malloc
    bench.run("std::malloc/free (64 bytes)", [&] {
        void* ptr = std::malloc(64);
        ankerl::nanobench::doNotOptimizeAway(ptr);
        std::free(ptr);
    });

    bench.run("std::malloc/free (1024 bytes)", [&] {
        void* ptr = std::malloc(1024);
        ankerl::nanobench::doNotOptimizeAway(ptr);
        std::free(ptr);
    });

    // Aligned allocation
    bench.run("alignedAlloc/Free (64 bytes, 64 align)", [&] {
        void* ptr = alignedAlloc(64, 64);
        ankerl::nanobench::doNotOptimizeAway(ptr);
        alignedFree(ptr);
    });

    bench.run("alignedAlloc/Free (1024 bytes, 64 align)", [&] {
        void* ptr = alignedAlloc(1024, 64);
        ankerl::nanobench::doNotOptimizeAway(ptr);
        alignedFree(ptr);
    });

    // Linear allocator
    {
        LinearAllocator arena(1024 * 1024);  // 1MB

        bench.run("LinearAllocator::alloc (64 bytes)", [&] {
            void* ptr = arena.alloc(64);
            ankerl::nanobench::doNotOptimizeAway(ptr);
        });

        arena.reset();

        bench.run("LinearAllocator::alloc (1024 bytes)", [&] {
            void* ptr = arena.alloc(1024);
            ankerl::nanobench::doNotOptimizeAway(ptr);
        });

        arena.reset();
    }

    // Pool allocator
    {
        PoolAllocator<64, 16> pool(10000);

        bench.run("PoolAllocator<64>::alloc", [&] {
            void* ptr = pool.alloc();
            ankerl::nanobench::doNotOptimizeAway(ptr);
            pool.free(ptr);
        });
    }

    // Bulk allocation comparison
    bench.run("std::vector<int> reserve 10000", [&] {
        std::vector<int> vec;
        vec.reserve(10000);
        ankerl::nanobench::doNotOptimizeAway(vec.data());
    });

    {
        LinearAllocator arena(10000 * sizeof(int) + 64);

        bench.run("LinearAllocator allocArray<int>(10000)", [&] {
            arena.reset();
            int* arr = arena.allocArray<int>(10000);
            ankerl::nanobench::doNotOptimizeAway(arr);
        });
    }

    return 0;
}
