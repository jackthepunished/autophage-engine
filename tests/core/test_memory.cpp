/// @file test_memory.cpp
/// @brief Tests for memory utilities

#include <autophage/core/memory.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace autophage;

TEST_CASE("Aligned allocation", "[core][memory]")
{
    SECTION("16-byte alignment")
    {
        void* ptr = alignedAlloc(1024, 16);
        REQUIRE(ptr != nullptr);
        REQUIRE(reinterpret_cast<uintptr_t>(ptr) % 16 == 0);
        alignedFree(ptr);
    }

    SECTION("64-byte alignment (cache line)")
    {
        void* ptr = alignedAlloc(1024, 64);
        REQUIRE(ptr != nullptr);
        REQUIRE(reinterpret_cast<uintptr_t>(ptr) % 64 == 0);
        alignedFree(ptr);
    }

    SECTION("256-byte alignment")
    {
        void* ptr = alignedAlloc(1024, 256);
        REQUIRE(ptr != nullptr);
        REQUIRE(reinterpret_cast<uintptr_t>(ptr) % 256 == 0);
        alignedFree(ptr);
    }
}

TEST_CASE("Cache-aligned allocation", "[core][memory]")
{
    void* ptr = cacheAlignedAlloc(1024);
    REQUIRE(ptr != nullptr);
    REQUIRE(reinterpret_cast<uintptr_t>(ptr) % AUTOPHAGE_CACHE_LINE_SIZE == 0);
    alignedFree(ptr);
}

TEST_CASE("LinearAllocator", "[core][memory]")
{
    LinearAllocator arena(1024, MemoryTag::Temporary);

    SECTION("Basic allocation")
    {
        void* ptr1 = arena.alloc(100);
        REQUIRE(ptr1 != nullptr);
        REQUIRE(arena.used() >= 100);

        void* ptr2 = arena.alloc(200);
        REQUIRE(ptr2 != nullptr);
        REQUIRE(arena.used() >= 300);
    }

    SECTION("Alignment")
    {
        void* ptr1 = arena.alloc(1, 1);  // 1 byte, 1 alignment
        REQUIRE(ptr1 != nullptr);

        void* ptr2 = arena.alloc(1, 64);  // 1 byte, 64 alignment
        REQUIRE(ptr2 != nullptr);
        REQUIRE(reinterpret_cast<uintptr_t>(ptr2) % 64 == 0);
    }

    SECTION("Reset clears allocations")
    {
        (void)arena.alloc(500);
        REQUIRE(arena.used() >= 500);

        arena.reset();
        REQUIRE(arena.used() == 0);
    }

    SECTION("Returns nullptr when full")
    {
        void* ptr1 = arena.alloc(1000);
        REQUIRE(ptr1 != nullptr);

        void* ptr2 = arena.alloc(100);
        REQUIRE(ptr2 == nullptr);
    }

    SECTION("Create objects")
    {
        struct TestStruct
        {
            int a;
            float b;
        };

        auto* obj = arena.create<TestStruct>(42, 3.14f);
        REQUIRE(obj != nullptr);
        REQUIRE(obj->a == 42);
        REQUIRE(obj->b == 3.14f);
    }

    SECTION("Allocate arrays")
    {
        int* arr = arena.allocArray<int>(10);
        REQUIRE(arr != nullptr);

        for (int i = 0; i < 10; ++i) {
            arr[i] = i;
        }

        for (int i = 0; i < 10; ++i) {
            REQUIRE(arr[i] == i);
        }
    }
}

TEST_CASE("PoolAllocator", "[core][memory]")
{
    PoolAllocator<64, 16> pool(100);

    SECTION("Allocation and free")
    {
        REQUIRE(pool.available() == 100);

        void* ptr1 = pool.alloc();
        REQUIRE(ptr1 != nullptr);
        REQUIRE(pool.allocated() == 1);

        void* ptr2 = pool.alloc();
        REQUIRE(ptr2 != nullptr);
        REQUIRE(pool.allocated() == 2);

        pool.free(ptr1);
        REQUIRE(pool.allocated() == 1);

        pool.free(ptr2);
        REQUIRE(pool.allocated() == 0);
    }

    SECTION("Create and destroy objects")
    {
        struct TestObj
        {
            int value;
            explicit TestObj(int v) : value(v) {}
        };

        auto* obj = pool.create<TestObj>(42);
        REQUIRE(obj != nullptr);
        REQUIRE(obj->value == 42);

        pool.destroy(obj);
        REQUIRE(pool.allocated() == 0);
    }

    SECTION("Reuses freed blocks")
    {
        void* ptr1 = pool.alloc();
        pool.free(ptr1);

        void* ptr2 = pool.alloc();
        REQUIRE(ptr2 == ptr1);  // Should reuse the same block
    }
}

TEST_CASE("MemoryTag toString", "[core][memory]")
{
    REQUIRE(toString(MemoryTag::Core) == "Core");
    REQUIRE(toString(MemoryTag::ECS) == "ECS");
    REQUIRE(toString(MemoryTag::Profiler) == "Profiler");
}
