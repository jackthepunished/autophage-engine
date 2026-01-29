#pragma once

/// @file platform.hpp
/// @brief Platform detection and abstraction for Autophage Engine

#include <autophage/core/types.hpp>

// =============================================================================
// Platform Detection
// =============================================================================

#if defined(_WIN32) || defined(_WIN64)
    #define AUTOPHAGE_PLATFORM_WINDOWS 1
    #define AUTOPHAGE_PLATFORM_NAME "Windows"
#elif defined(__linux__)
    #define AUTOPHAGE_PLATFORM_LINUX 1
    #define AUTOPHAGE_PLATFORM_NAME "Linux"
#elif defined(__APPLE__) && defined(__MACH__)
    #define AUTOPHAGE_PLATFORM_MACOS 1
    #define AUTOPHAGE_PLATFORM_NAME "macOS"
#else
    #error "Unsupported platform"
#endif

// =============================================================================
// Compiler Detection
// =============================================================================

#if defined(_MSC_VER)
    #define AUTOPHAGE_COMPILER_MSVC 1
    #define AUTOPHAGE_COMPILER_NAME "MSVC"
    #define AUTOPHAGE_COMPILER_VERSION _MSC_VER
#elif defined(__clang__)
    #define AUTOPHAGE_COMPILER_CLANG 1
    #define AUTOPHAGE_COMPILER_NAME "Clang"
    #define AUTOPHAGE_COMPILER_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100)
#elif defined(__GNUC__)
    #define AUTOPHAGE_COMPILER_GCC 1
    #define AUTOPHAGE_COMPILER_NAME "GCC"
    #define AUTOPHAGE_COMPILER_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100)
#else
    #error "Unsupported compiler"
#endif

// =============================================================================
// Architecture Detection
// =============================================================================

#if defined(_M_X64) || defined(__x86_64__)
    #define AUTOPHAGE_ARCH_X64 1
    #define AUTOPHAGE_ARCH_NAME "x64"
#elif defined(_M_ARM64) || defined(__aarch64__)
    #define AUTOPHAGE_ARCH_ARM64 1
    #define AUTOPHAGE_ARCH_NAME "ARM64"
#elif defined(_M_IX86) || defined(__i386__)
    #define AUTOPHAGE_ARCH_X86 1
    #define AUTOPHAGE_ARCH_NAME "x86"
#else
    #error "Unsupported architecture"
#endif

// =============================================================================
// SIMD Detection
// =============================================================================

#if defined(__AVX512F__)
    #define AUTOPHAGE_SIMD_AVX512 1
    #define AUTOPHAGE_SIMD_LEVEL 512
#elif defined(__AVX2__)
    #define AUTOPHAGE_SIMD_AVX2 1
    #define AUTOPHAGE_SIMD_LEVEL 256
#elif defined(__AVX__)
    #define AUTOPHAGE_SIMD_AVX 1
    #define AUTOPHAGE_SIMD_LEVEL 256
#elif defined(__SSE4_2__)
    #define AUTOPHAGE_SIMD_SSE42 1
    #define AUTOPHAGE_SIMD_LEVEL 128
#elif defined(__SSE2__) || defined(_M_X64)
    #define AUTOPHAGE_SIMD_SSE2 1
    #define AUTOPHAGE_SIMD_LEVEL 128
#elif defined(__ARM_NEON)
    #define AUTOPHAGE_SIMD_NEON 1
    #define AUTOPHAGE_SIMD_LEVEL 128
#else
    #define AUTOPHAGE_SIMD_LEVEL 0
#endif

// =============================================================================
// Build Configuration
// =============================================================================

#if defined(NDEBUG)
    #define AUTOPHAGE_BUILD_RELEASE 1
    #define AUTOPHAGE_BUILD_NAME "Release"
#else
    #define AUTOPHAGE_BUILD_DEBUG 1
    #define AUTOPHAGE_BUILD_NAME "Debug"
#endif

// =============================================================================
// Compiler Attributes
// =============================================================================

// Force inline
#if defined(AUTOPHAGE_COMPILER_MSVC)
    #define AUTOPHAGE_FORCE_INLINE __forceinline
#else
    #define AUTOPHAGE_FORCE_INLINE __attribute__((always_inline)) inline
#endif

// No inline
#if defined(AUTOPHAGE_COMPILER_MSVC)
    #define AUTOPHAGE_NO_INLINE __declspec(noinline)
#else
    #define AUTOPHAGE_NO_INLINE __attribute__((noinline))
#endif

// Export/Import symbols
#if defined(AUTOPHAGE_PLATFORM_WINDOWS)
    #if defined(AUTOPHAGE_EXPORT)
        #define AUTOPHAGE_API __declspec(dllexport)
    #elif defined(AUTOPHAGE_IMPORT)
        #define AUTOPHAGE_API __declspec(dllimport)
    #else
        #define AUTOPHAGE_API
    #endif
#else
    #if defined(AUTOPHAGE_EXPORT)
        #define AUTOPHAGE_API __attribute__((visibility("default")))
    #else
        #define AUTOPHAGE_API
    #endif
#endif

// Likely/Unlikely branch hints
#if defined(__cpp_attributes) && __cpp_attributes >= 201803L
    #define AUTOPHAGE_LIKELY [[likely]]
    #define AUTOPHAGE_UNLIKELY [[unlikely]]
#else
    #define AUTOPHAGE_LIKELY
    #define AUTOPHAGE_UNLIKELY
#endif

// Restrict pointer
#if defined(AUTOPHAGE_COMPILER_MSVC)
    #define AUTOPHAGE_RESTRICT __restrict
#else
    #define AUTOPHAGE_RESTRICT __restrict__
#endif

// Alignment
#define AUTOPHAGE_ALIGN(x) alignas(x)
#define AUTOPHAGE_CACHE_LINE_SIZE 64
#define AUTOPHAGE_CACHE_ALIGNED AUTOPHAGE_ALIGN(AUTOPHAGE_CACHE_LINE_SIZE)

// =============================================================================
// Debug Break
// =============================================================================

#if defined(AUTOPHAGE_COMPILER_MSVC)
    #define AUTOPHAGE_DEBUG_BREAK() __debugbreak()
#elif defined(AUTOPHAGE_COMPILER_CLANG) || defined(AUTOPHAGE_COMPILER_GCC)
    #if defined(AUTOPHAGE_ARCH_X64) || defined(AUTOPHAGE_ARCH_X86)
        #define AUTOPHAGE_DEBUG_BREAK() __asm__ volatile("int $0x03")
    #elif defined(AUTOPHAGE_ARCH_ARM64)
        #define AUTOPHAGE_DEBUG_BREAK() __builtin_trap()
    #else
        #define AUTOPHAGE_DEBUG_BREAK() __builtin_trap()
    #endif
#else
    #define AUTOPHAGE_DEBUG_BREAK() ((void)0)
#endif

namespace autophage {

/// @brief Platform information structure
struct PlatformInfo {
    StringView name;
    StringView compiler;
    StringView arch;
    StringView build;
    u32 compilerVersion;
    u32 simdLevel;
    usize cacheLineSize;
};

/// @brief Get platform information
[[nodiscard]] inline constexpr PlatformInfo getPlatformInfo() noexcept {
    return PlatformInfo{
        .name = AUTOPHAGE_PLATFORM_NAME,
        .compiler = AUTOPHAGE_COMPILER_NAME,
        .arch = AUTOPHAGE_ARCH_NAME,
        .build = AUTOPHAGE_BUILD_NAME,
        .compilerVersion = AUTOPHAGE_COMPILER_VERSION,
        .simdLevel = AUTOPHAGE_SIMD_LEVEL,
        .cacheLineSize = AUTOPHAGE_CACHE_LINE_SIZE,
    };
}

}  // namespace autophage
