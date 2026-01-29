#pragma once

/// @file logger.hpp
/// @brief Logging interface for Autophage Engine (spdlog wrapper)

#include <autophage/core/types.hpp>

#include <spdlog/spdlog.h>

#include <memory>

namespace autophage {

// =============================================================================
// Log Levels
// =============================================================================

enum class LogLevel : u8
{
    Trace = 0,
    Debug,
    Info,
    Warn,
    Error,
    Fatal,
    Off,
};

// =============================================================================
// Logger Interface
// =============================================================================

/// @brief Initialize the logging system
/// @param appName Application name for log identification
/// @param level Minimum log level to output
void initLogger(StringView appName, LogLevel level = LogLevel::Info);

/// @brief Shutdown the logging system
void shutdownLogger();

/// @brief Set the minimum log level
void setLogLevel(LogLevel level);

/// @brief Get the current log level
[[nodiscard]] LogLevel getLogLevel();

/// @brief Flush all log buffers
void flushLogs();

// =============================================================================
// Logging Functions
// =============================================================================

/// @brief Log a trace message
template <typename... Args>
inline void logTrace(spdlog::format_string_t<Args...> fmt, Args&&... args)
{
    spdlog::trace(fmt, std::forward<Args>(args)...);
}

/// @brief Log a debug message
template <typename... Args>
inline void logDebug(spdlog::format_string_t<Args...> fmt, Args&&... args)
{
    spdlog::debug(fmt, std::forward<Args>(args)...);
}

/// @brief Log an info message
template <typename... Args>
inline void logInfo(spdlog::format_string_t<Args...> fmt, Args&&... args)
{
    spdlog::info(fmt, std::forward<Args>(args)...);
}

/// @brief Log a warning message
template <typename... Args>
inline void logWarn(spdlog::format_string_t<Args...> fmt, Args&&... args)
{
    spdlog::warn(fmt, std::forward<Args>(args)...);
}

/// @brief Log an error message
template <typename... Args>
inline void logError(spdlog::format_string_t<Args...> fmt, Args&&... args)
{
    spdlog::error(fmt, std::forward<Args>(args)...);
}

/// @brief Log a fatal message (used by assert system)
void logFatal(const char* file, int line, const char* func, const char* msg);

// =============================================================================
// Scoped Log Context
// =============================================================================

/// @brief RAII helper for scoped log context
class ScopedLogContext
{
public:
    explicit ScopedLogContext(StringView context);
    ~ScopedLogContext();

    ScopedLogContext(const ScopedLogContext&) = delete;
    ScopedLogContext& operator=(const ScopedLogContext&) = delete;

private:
    StringView previousContext_;
};

/// @brief Set the current log context
void setLogContext(StringView context);

/// @brief Get the current log context
[[nodiscard]] StringView getLogContext();

// =============================================================================
// Macros for Convenience
// =============================================================================

#define LOG_TRACE(...) ::autophage::logTrace(__VA_ARGS__)
#define LOG_DEBUG(...) ::autophage::logDebug(__VA_ARGS__)
#define LOG_INFO(...) ::autophage::logInfo(__VA_ARGS__)
#define LOG_WARN(...) ::autophage::logWarn(__VA_ARGS__)
#define LOG_ERROR(...) ::autophage::logError(__VA_ARGS__)

#define LOG_SCOPE(name) ::autophage::ScopedLogContext _log_scope_##__LINE__(name)

}  // namespace autophage
