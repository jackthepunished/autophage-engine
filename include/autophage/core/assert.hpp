#pragma once

/// @file assert.hpp
/// @brief Assertion macros with logging support for Autophage Engine

#include <autophage/core/platform.hpp>

#include <cstdlib>

// Forward declare logger
namespace autophage {
void logFatal(const char* file, int line, const char* func, const char* msg);
}

// =============================================================================
// Assert Macros
// =============================================================================

/// @brief Always-on assertion (even in release builds)
#define AUTOPHAGE_ASSERT_ALWAYS(condition, message)                                         \
    do {                                                                                     \
        if (!(condition)) AUTOPHAGE_UNLIKELY {                                               \
            ::autophage::logFatal(__FILE__, __LINE__, __func__, message);                    \
            AUTOPHAGE_DEBUG_BREAK();                                                         \
            std::abort();                                                                    \
        }                                                                                    \
    } while (false)

/// @brief Debug-only assertion (stripped in release)
#if defined(AUTOPHAGE_BUILD_DEBUG)
    #define AUTOPHAGE_ASSERT(condition, message) AUTOPHAGE_ASSERT_ALWAYS(condition, message)
#else
    #define AUTOPHAGE_ASSERT(condition, message) ((void)0)
#endif

/// @brief Debug-only assertion with expression evaluation
#if defined(AUTOPHAGE_BUILD_DEBUG)
    #define AUTOPHAGE_ASSERT_DEBUG(condition) AUTOPHAGE_ASSERT(condition, #condition)
#else
    #define AUTOPHAGE_ASSERT_DEBUG(condition) ((void)0)
#endif

/// @brief Verify macro (always evaluates, asserts in debug)
#if defined(AUTOPHAGE_BUILD_DEBUG)
    #define AUTOPHAGE_VERIFY(condition)                                                      \
        do {                                                                                  \
            if (!(condition)) AUTOPHAGE_UNLIKELY {                                            \
                ::autophage::logFatal(__FILE__, __LINE__, __func__, #condition " failed");    \
                AUTOPHAGE_DEBUG_BREAK();                                                      \
                std::abort();                                                                 \
            }                                                                                 \
        } while (false)
#else
    #define AUTOPHAGE_VERIFY(condition) (void)(condition)
#endif

/// @brief Unreachable code marker
#if defined(AUTOPHAGE_COMPILER_MSVC)
    #define AUTOPHAGE_UNREACHABLE()                                                          \
        do {                                                                                  \
            AUTOPHAGE_ASSERT_ALWAYS(false, "Unreachable code reached");                       \
            __assume(0);                                                                      \
        } while (false)
#else
    #define AUTOPHAGE_UNREACHABLE()                                                          \
        do {                                                                                  \
            AUTOPHAGE_ASSERT_ALWAYS(false, "Unreachable code reached");                       \
            __builtin_unreachable();                                                          \
        } while (false)
#endif

/// @brief Not implemented marker
#define AUTOPHAGE_NOT_IMPLEMENTED()                                                          \
    do {                                                                                      \
        ::autophage::logFatal(__FILE__, __LINE__, __func__, "Not implemented");               \
        AUTOPHAGE_DEBUG_BREAK();                                                              \
        std::abort();                                                                         \
    } while (false)

// =============================================================================
// Precondition/Postcondition
// =============================================================================

/// @brief Function precondition check
#define AUTOPHAGE_PRECONDITION(condition) \
    AUTOPHAGE_ASSERT(condition, "Precondition failed: " #condition)

/// @brief Function postcondition check
#define AUTOPHAGE_POSTCONDITION(condition) \
    AUTOPHAGE_ASSERT(condition, "Postcondition failed: " #condition)

/// @brief Invariant check
#define AUTOPHAGE_INVARIANT(condition) \
    AUTOPHAGE_ASSERT(condition, "Invariant violated: " #condition)
