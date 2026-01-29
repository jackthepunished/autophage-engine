/// @file logger.cpp
/// @brief Logging implementation using spdlog

#include <autophage/core/logger.hpp>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <mutex>

namespace autophage {

namespace {

// Global logger instance
std::shared_ptr<spdlog::logger> g_logger;
std::atomic<LogLevel> g_logLevel{LogLevel::Info};

// Thread-local context
thread_local std::string g_logContext;

// Convert our log level to spdlog level
spdlog::level::level_enum toSpdlogLevel(LogLevel level)
{
    switch (level) {
        case LogLevel::Trace:
            return spdlog::level::trace;
        case LogLevel::Debug:
            return spdlog::level::debug;
        case LogLevel::Info:
            return spdlog::level::info;
        case LogLevel::Warn:
            return spdlog::level::warn;
        case LogLevel::Error:
            return spdlog::level::err;
        case LogLevel::Fatal:
            return spdlog::level::critical;
        case LogLevel::Off:
            return spdlog::level::off;
    }
    return spdlog::level::info;
}

}  // namespace

void initLogger(StringView appName, LogLevel level)
{
    try {
        // Create console sink with colors
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_level(toSpdlogLevel(level));

        // Create file sink
        std::string logFileName = std::string(appName) + ".log";
        auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFileName, true);
        fileSink->set_level(spdlog::level::trace);  // File gets everything

        // Create multi-sink logger
        std::vector<spdlog::sink_ptr> sinks{consoleSink, fileSink};
        g_logger =
            std::make_shared<spdlog::logger>(std::string(appName), sinks.begin(), sinks.end());
        g_logger->set_level(toSpdlogLevel(level));

        // Set pattern: [timestamp] [level] [context] message
        g_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

        // Register as default logger
        spdlog::set_default_logger(g_logger);

        g_logLevel.store(level, std::memory_order_relaxed);

        g_logger->info("Logger initialized: {} (level: {})", appName, static_cast<int>(level));
    } catch (const spdlog::spdlog_ex& ex) {
        // Fallback to stderr if logger setup fails
        fprintf(stderr, "Logger initialization failed: %s\n", ex.what());
    }
}

void shutdownLogger()
{
    if (g_logger) {
        g_logger->info("Logger shutting down");
        g_logger->flush();
        g_logger.reset();
    }
    spdlog::shutdown();
}

void setLogLevel(LogLevel level)
{
    g_logLevel.store(level, std::memory_order_relaxed);
    if (g_logger) {
        g_logger->set_level(toSpdlogLevel(level));
    }
}

LogLevel getLogLevel()
{
    return g_logLevel.load(std::memory_order_relaxed);
}

void flushLogs()
{
    if (g_logger) {
        g_logger->flush();
    }
}

void logFatal(const char* file, int line, const char* func, const char* msg)
{
    if (g_logger) {
        g_logger->critical("[FATAL] {}:{} in {}: {}", file, line, func, msg);
        g_logger->flush();
    } else {
        fprintf(stderr, "[FATAL] %s:%d in %s: %s\n", file, line, func, msg);
    }
}

// Template implementations
// Template implementations moved to header

// Context management
ScopedLogContext::ScopedLogContext(StringView context) : previousContext_(g_logContext)
{
    g_logContext = context;
}

ScopedLogContext::~ScopedLogContext()
{
    g_logContext = previousContext_;
}

void setLogContext(StringView context)
{
    g_logContext = context;
}

StringView getLogContext()
{
    return g_logContext;
}

}  // namespace autophage
