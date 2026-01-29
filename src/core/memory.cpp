/// @file memory.cpp
/// @brief Memory utilities implementation

#include <autophage/core/assert.hpp>
#include <autophage/core/memory.hpp>

#include <array>
#include <atomic>
#include <mutex>

namespace autophage {

namespace {

// Memory tracking data
struct TaggedMemoryTracker
{
    std::atomic<usize> currentBytes{0};
    std::atomic<usize> peakBytes{0};
    std::atomic<u64> totalAllocations{0};
    std::atomic<u64> totalDeallocations{0};
};

std::array<TaggedMemoryTracker, static_cast<usize>(MemoryTag::Count)> g_memoryTrackers;

void trackAllocation(MemoryTag tag, usize size)
{
    auto& tracker = g_memoryTrackers[static_cast<usize>(tag)];
    usize current = tracker.currentBytes.fetch_add(size, std::memory_order_relaxed) + size;

    // Update peak (lock-free)
    usize peak = tracker.peakBytes.load(std::memory_order_relaxed);
    while (current > peak &&
           !tracker.peakBytes.compare_exchange_weak(peak, current, std::memory_order_relaxed)) {
        // Retry
    }

    tracker.totalAllocations.fetch_add(1, std::memory_order_relaxed);
}

void trackDeallocation(MemoryTag tag, usize size)
{
    auto& tracker = g_memoryTrackers[static_cast<usize>(tag)];
    tracker.currentBytes.fetch_sub(size, std::memory_order_relaxed);
    tracker.totalDeallocations.fetch_add(1, std::memory_order_relaxed);
}

}  // namespace

// =============================================================================
// Memory Statistics
// =============================================================================

MemoryStats getMemoryStats(MemoryTag tag) noexcept
{
    const auto& tracker = g_memoryTrackers[static_cast<usize>(tag)];
    return MemoryStats{
        .currentBytes = tracker.currentBytes.load(std::memory_order_relaxed),
        .peakBytes = tracker.peakBytes.load(std::memory_order_relaxed),
        .totalAllocations = tracker.totalAllocations.load(std::memory_order_relaxed),
        .totalDeallocations = tracker.totalDeallocations.load(std::memory_order_relaxed),
    };
}

MemoryStats getTotalMemoryStats() noexcept
{
    MemoryStats total{};
    for (usize i = 0; i < static_cast<usize>(MemoryTag::Count); ++i) {
        const auto& tracker = g_memoryTrackers[i];
        total.currentBytes += tracker.currentBytes.load(std::memory_order_relaxed);
        total.peakBytes += tracker.peakBytes.load(std::memory_order_relaxed);
        total.totalAllocations += tracker.totalAllocations.load(std::memory_order_relaxed);
        total.totalDeallocations += tracker.totalDeallocations.load(std::memory_order_relaxed);
    }
    return total;
}

void resetMemoryStats() noexcept
{
    for (auto& tracker : g_memoryTrackers) {
        tracker.currentBytes.store(0, std::memory_order_relaxed);
        tracker.peakBytes.store(0, std::memory_order_relaxed);
        tracker.totalAllocations.store(0, std::memory_order_relaxed);
        tracker.totalDeallocations.store(0, std::memory_order_relaxed);
    }
}

// =============================================================================
// Tagged Allocation
// =============================================================================

void* taggedAlloc(usize size, MemoryTag tag) noexcept
{
    void* ptr = std::malloc(size);
    if (ptr) {
        trackAllocation(tag, size);
    }
    return ptr;
}

void* taggedAlignedAlloc(usize size, usize alignment, MemoryTag tag) noexcept
{
    void* ptr = alignedAlloc(size, alignment);
    if (ptr) {
        trackAllocation(tag, size);
    }
    return ptr;
}

void taggedAlignedFree(void* ptr, MemoryTag tag) noexcept
{
    if (ptr) {
        trackDeallocation(tag, 0);
        alignedFree(ptr);
    }
}

void taggedFree(void* ptr, MemoryTag tag) noexcept
{
    if (ptr) {
        // Note: We don't know the size here, so we track 0
        // In a production system, we'd store size in a header or use a size map
        trackDeallocation(tag, 0);
        std::free(ptr);
    }
}

// =============================================================================
// Linear Allocator
// =============================================================================

LinearAllocator::LinearAllocator(usize capacity, MemoryTag tag) : capacity_(capacity), tag_(tag)
{
    memory_ = static_cast<Byte*>(taggedAlignedAlloc(capacity, AUTOPHAGE_CACHE_LINE_SIZE, tag));
    AUTOPHAGE_ASSERT(memory_ != nullptr, "Failed to allocate linear allocator memory");
}

LinearAllocator::~LinearAllocator()
{
    if (memory_) {}
}

LinearAllocator::LinearAllocator(LinearAllocator&& other) noexcept
    : memory_(other.memory_), capacity_(other.capacity_), offset_(other.offset_), tag_(other.tag_)
{
    other.memory_ = nullptr;
    other.capacity_ = 0;
    other.offset_ = 0;
}

LinearAllocator& LinearAllocator::operator=(LinearAllocator&& other) noexcept
{
    if (this != &other) {
        if (memory_) {
            taggedAlignedFree(memory_, tag_);
        }
        memory_ = other.memory_;
        capacity_ = other.capacity_;
        offset_ = other.offset_;
        tag_ = other.tag_;

        other.memory_ = nullptr;
        other.capacity_ = 0;
        other.offset_ = 0;
    }
    return *this;
}

void* LinearAllocator::alloc(usize size, usize alignment) noexcept
{
    // Align the current offset
    usize alignedOffset = (offset_ + alignment - 1) & ~(alignment - 1);

    // Check if we have enough space
    if (alignedOffset + size > capacity_) {
        return nullptr;
    }

    void* ptr = memory_ + alignedOffset;
    offset_ = alignedOffset + size;
    return ptr;
}

void LinearAllocator::reset() noexcept
{
    offset_ = 0;
}

}  // namespace autophage
