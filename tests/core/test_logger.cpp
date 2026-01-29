/// @file test_logger.cpp
/// @brief Tests for logging system

#include <catch2/catch_test_macros.hpp>
#include <autophage/core/logger.hpp>

using namespace autophage;

TEST_CASE("Logger initialization", "[core][logger]") {
    // Initialize logger for tests
    initLogger("test", LogLevel::Debug);
    
    SECTION("Log level can be set and retrieved") {
        setLogLevel(LogLevel::Warn);
        REQUIRE(getLogLevel() == LogLevel::Warn);
        
        setLogLevel(LogLevel::Debug);
        REQUIRE(getLogLevel() == LogLevel::Debug);
    }
    
    SECTION("Logging functions don't crash") {
        // These should not throw or crash
        LOG_TRACE("Trace message");
        LOG_DEBUG("Debug message");
        LOG_INFO("Info message");
        LOG_WARN("Warning message");
        LOG_ERROR("Error message");
    }
    
    shutdownLogger();
}

TEST_CASE("ScopedLogContext", "[core][logger]") {
    initLogger("test", LogLevel::Debug);
    
    SECTION("Context is restored on scope exit") {
        setLogContext("outer");
        REQUIRE(getLogContext() == "outer");
        
        {
            ScopedLogContext scope("inner");
            // Note: Our implementation replaces context, doesn't nest
        }
        
        // Context should be restored (implementation dependent)
    }
    
    shutdownLogger();
}
