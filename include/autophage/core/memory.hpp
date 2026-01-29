#pragma once

/// @file memory.hpp
/// @brief Memory utilities and allocators for Autophage Engine

#include <autophage/core/platform.hpp>
#include <autophage/core/types.hpp>

#include <cstdlib>
#include <cstring>
#include <new>

namespace autophage {

// =============================================================================
// Aligned Memory Operations
// =============================================================================

/// @brief Allocate aligned memory
/// @param size Size in bytes to allocate
/// @param alignment Alignment requirement (must be power of 2)
/// @return Pointer to allocated memory, or nullptr on failure
[[nodiscard]] inline void* alignedAlloc(usize size, usize alignment) noexcept
{
#if defined(AUTOPHAGE_PLATFORM_WINDOWS)
    return _aligned_malloc(size, alignment);
#else
    void* ptr = nullptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return nullptr;
    }
    return ptr;
#endif
}

/// @brief Free aligned memory
/// @param ptr Pointer to free (may be nullptr)
inline void alignedFree(void* ptr) noexcept
{
#if defined(AUTOPHAGE_PLATFORM_WINDOWS)
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

/// @brief Allocate cache-line aligned memory
[[nodiscard]] inline void* cacheAlignedAlloc(usize size) noexcept
{
    return alignedAlloc(size, AUTOPHAGE_CACHE_LINE_SIZE);
}

// =============================================================================
// Memory Tags for Tracking
// =============================================================================

/// @brief Memory allocation tags for profiling
enum class MemoryTag : u16
{
    Unknown = 0,
    Core,
    ECS,
    Components,
    Entities,
    Systems,
    Profiler,
    Renderer,
    Physics,
    Audio,
    Scripting,
    Temporary,
    Debug,

    Count
};

/// @brief Convert memory tag to string
[[nodiscard]] constexpr StringView toString(MemoryTag tag) noexcept
{
    switch (tag) {
        case MemoryTag::Unknown:
            return "Unknown";
        case MemoryTag::Core:
            return "Core";
        case MemoryTag::ECS:
            return "ECS";
        case MemoryTag::Components:
            return "Components";
        case MemoryTag::Entities:
            return "Entities";
        case MemoryTag::Systems:
            return "Systems";
        case MemoryTag::Profiler:
            return "Profiler";
        case MemoryTag::Renderer:
            return "Renderer";
        case MemoryTag::Physics:
            return "Physics";
        case MemoryTag::Audio:
            return "Audio";
        case MemoryTag::Scripting:
            return "Scripting";
        case MemoryTag::Temporary:
            return "Temporary";
        case MemoryTag::Debug:
            return "Debug";
        case MemoryTag::Count:
            return "Count";
    }
    return "Unknown";
}

// =============================================================================
// Memory Statistics
// =============================================================================

/// @brief Memory allocation statistics per tag
struct MemoryStats
{
    usize currentBytes = 0;
    usize peakBytes = 0;
    u64 totalAllocations = 0;
    u64 totalDeallocations = 0;
};

/// @brief Get memory statistics for a specific tag
[[nodiscard]] MemoryStats getMemoryStats(MemoryTag tag) noexcept;

/// @brief Get total memory statistics across all tags
[[nodiscard]] MemoryStats getTotalMemoryStats() noexcept;

/// @brief Reset memory statistics
void resetMemoryStats() noexcept;

// =============================================================================
// Tagged Allocation Functions
// =============================================================================

/// @brief Allocate memory with tag for tracking
[[nodiscard]] void* taggedAlloc(usize size, MemoryTag tag) noexcept;

/// @brief Allocate aligned memory with tag for tracking
[[nodiscard]] void* taggedAlignedAlloc(usize size, usize alignment, MemoryTag tag) noexcept;

/// @brief Free tagged aligned memory
void taggedAlignedFree(void* ptr, MemoryTag tag) noexcept;

/// @brief Free tagged memory
void taggedFree(void* ptr, MemoryTag tag) noexcept;

// =============================================================================
// Linear Allocator (Arena)
// =============================================================================

/// @brief Fast linear allocator for temporary allocations
/// @note Memory is freed all at once when reset() is called
class LinearAllocator
{
public:
    /// @brief Create a linear allocator with the given capacity
    explicit LinearAllocator(usize capacity, MemoryTag tag = MemoryTag::Temporary);

    ~LinearAllocator();

    // Non-copyable, moveable
    LinearAllocator(const LinearAllocator&) = delete;
    LinearAllocator& operator=(const LinearAllocator&) = delete;
    LinearAllocator(LinearAllocator&& other) noexcept;
    LinearAllocator& operator=(LinearAllocator&& other) noexcept;

    /// @brief Allocate memory from the arena
    /// @param size Size in bytes
    /// @param alignment Alignment requirement (default 16)
    /// @return Pointer to allocated memory, or nullptr if out of space
    [[nodiscard]] void* alloc(usize size, usize alignment = 16) noexcept;

    /// @brief Allocate and construct an object
    template <typename T, typename... Args> [[nodiscard]] T* create(Args&&... args)
    {
        void* ptr = alloc(sizeof(T), alignof(T));
        if (!ptr)
            return nullptr;
        return new (ptr) T(std::forward<Args>(args)...);
    }

    /// @brief Allocate an array
    template <typename T> [[nodiscard]] T* allocArray(usize count) noexcept
    {
        return static_cast<T*>(alloc(sizeof(T) * count, alignof(T)));
    }

    /// @brief Reset the allocator (frees all allocations)
    /// @param offset If specified, resets to this offset instead of 0
    void reset(usize offset = 0) noexcept;

    /// @brief Get current usage
    [[nodiscard]] usize used() const noexcept { return offset_; }

    /// @brief Get total capacity
    [[nodiscard]] usize capacity() const noexcept { return capacity_; }

    /// @brief Get remaining space
    [[nodiscard]] usize remaining() const noexcept { return capacity_ - offset_; }

private:
    Byte* memory_ = nullptr;
    usize capacity_ = 0;
    usize offset_ = 0;
    MemoryTag tag_;
};

// =============================================================================
// Pool Allocator
// =============================================================================

/// @brief Fixed-size block pool allocator
/// @tparam BlockSize Size of each block
/// @tparam Alignment Alignment of each block
template <usize BlockSize, usize Alignment = alignof(std::max_align_t)> class PoolAllocator
{
    static_assert(BlockSize >= sizeof(void*), "Block size must be at least pointer size");
    static_assert((Alignment & (Alignment - 1)) == 0, "Alignment must be power of 2");

public:
    /// @brief Create a pool with the given number of blocks
    explicit PoolAllocator(usize blockCount, MemoryTag tag = MemoryTag::Core) : tag_(tag)
    {
        // Allocate memory for all blocks
        usize alignedBlockSize = (BlockSize + Alignment - 1) & ~(Alignment - 1);
        memory_ =
            static_cast<Byte*>(taggedAlignedAlloc(alignedBlockSize * blockCount, Alignment, tag));

        if (memory_) {
            capacity_ = blockCount;
            // Build free list
            for (usize i = 0; i < blockCount - 1; ++i) {
                Byte* current = memory_ + i * alignedBlockSize;
                Byte* next = memory_ + (i + 1) * alignedBlockSize;
                *reinterpret_cast<void**>(current) = next;
            }
            // Last block points to nullptr
            *reinterpret_cast<void**>(memory_ + (blockCount - 1) * alignedBlockSize) = nullptr;
            freeList_ = memory_;
        }
    }

    ~PoolAllocator()
    {
        if (memory_) {
            taggedAlignedFree(memory_, tag_);
        }
    }

    // Non-copyable
    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;

    /// @brief Allocate a block from the pool
    [[nodiscard]] void* alloc() noexcept
    {
        if (!freeList_)
            return nullptr;

        void* block = freeList_;
        freeList_ = *reinterpret_cast<void**>(freeList_);
        ++allocatedCount_;
        return block;
    }

    /// @brief Return a block to the pool
    void free(void* ptr) noexcept
    {
        if (!ptr)
            return;

        *reinterpret_cast<void**>(ptr) = freeList_;
        freeList_ = ptr;
        --allocatedCount_;
    }

    /// @brief Allocate and construct an object
    template <typename T, typename... Args> [[nodiscard]] T* create(Args&&... args)
    {
        static_assert(sizeof(T) <= BlockSize, "Type too large for pool");
        static_assert(alignof(T) <= Alignment, "Type alignment exceeds pool alignment");

        void* ptr = alloc();
        if (!ptr)
            return nullptr;
        return new (ptr) T(std::forward<Args>(args)...);
    }

    /// @brief Destroy and return an object
    template <typename T> void destroy(T* ptr)
    {
        if (ptr) {
            ptr->~T();
            free(ptr);
        }
    }

    [[nodiscard]] usize allocated() const noexcept { return allocatedCount_; }
    [[nodiscard]] usize capacity() const noexcept { return capacity_; }
    [[nodiscard]] usize available() const noexcept { return capacity_ - allocatedCount_; }

private:
    Byte* memory_ = nullptr;
    void* freeList_ = nullptr;
    usize capacity_ = 0;
    usize allocatedCount_ = 0;
    MemoryTag tag_;
};

// =============================================================================
// Scope-based temporary allocator
// =============================================================================

/// @brief RAII wrapper for temporary allocations that reset on scope exit
class ScopedAllocator
{
public:
    explicit ScopedAllocator(LinearAllocator& allocator)
        : allocator_(allocator), savedOffset_(allocator.used())
    {}

    ~ScopedAllocator() { allocator_.reset(savedOffset_); }

    [[nodiscard]] void* alloc(usize size, usize alignment = 16) noexcept
    {
        return allocator_.alloc(size, alignment);
    }

    template <typename T, typename... Args> [[nodiscard]] T* create(Args&&... args)
    {
        return allocator_.create<T>(std::forward<Args>(args)...);
    }

    template <typename T> [[nodiscard]] T* allocArray(usize count) noexcept
    {
        return allocator_.allocArray<T>(count);
    }

private:
    LinearAllocator& allocator_;
    usize savedOffset_;
};

}  // namespace autophage
