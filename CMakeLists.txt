# ==============================================================================
# Autophage Engine - Root CMake Configuration
# Self-Rewriting, Runtime-Adaptive Game Engine
# ==============================================================================

cmake_minimum_required(VERSION 3.20)

# Project definition
project(autophage
    VERSION 0.1.0
    DESCRIPTION "Self-Rewriting, Runtime-Adaptive Game Engine"
    HOMEPAGE_URL "https://github.com/autophage/autophage-engine"
    LANGUAGES CXX
)

# ==============================================================================
# Build Options
# ==============================================================================

option(AUTOPHAGE_BUILD_TESTS "Build unit tests" ON)
option(AUTOPHAGE_BUILD_BENCHMARKS "Build performance benchmarks" ON)
option(AUTOPHAGE_BUILD_EXAMPLES "Build example applications" ON)
option(AUTOPHAGE_BUILD_DOCS "Build documentation" OFF)
option(AUTOPHAGE_ENABLE_ASAN "Enable Address Sanitizer" OFF)
option(AUTOPHAGE_ENABLE_UBSAN "Enable Undefined Behavior Sanitizer" OFF)
option(AUTOPHAGE_USE_LLVM_JIT "Enable LLVM JIT support" OFF)

# ==============================================================================
# Global Configuration
# ==============================================================================

# C++23 required
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE/tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Position independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ==============================================================================
# Compiler Warnings and Flags
# ==============================================================================

add_library(autophage_warnings INTERFACE)

if(MSVC)
    target_compile_options(autophage_warnings INTERFACE
        /W4                 # High warning level
        /WX                 # Warnings as errors
        /permissive-        # Strict conformance
        /Zc:__cplusplus     # Correct __cplusplus macro
        /Zc:preprocessor    # Standard-conforming preprocessor
        /utf-8              # UTF-8 source and execution
        /EHsc               # Standard exception handling
        /wd4324             # Suppress warning: structure was padded due to alignment specifier
    )
    target_compile_definitions(autophage_warnings INTERFACE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
    )
else()
    target_compile_options(autophage_warnings INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wconversion
        -Wsign-conversion
        -Wcast-align
        -Wunused
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
    )
    
    # GCC-specific warnings
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(autophage_warnings INTERFACE
            -Wduplicated-cond
            -Wduplicated-branches
            -Wlogical-op
        )
    endif()
endif()

# ==============================================================================
# Sanitizers
# ==============================================================================

add_library(autophage_sanitizers INTERFACE)

if(AUTOPHAGE_ENABLE_ASAN)
    if(MSVC)
        target_compile_options(autophage_sanitizers INTERFACE /fsanitize=address)
    else()
        target_compile_options(autophage_sanitizers INTERFACE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(autophage_sanitizers INTERFACE -fsanitize=address)
    endif()
endif()

if(AUTOPHAGE_ENABLE_UBSAN AND NOT MSVC)
    target_compile_options(autophage_sanitizers INTERFACE -fsanitize=undefined)
    target_link_options(autophage_sanitizers INTERFACE -fsanitize=undefined)
endif()

# ==============================================================================
# Common Interface Library
# ==============================================================================

add_library(autophage_common INTERFACE)
target_link_libraries(autophage_common INTERFACE 
    autophage_warnings
    autophage_sanitizers
)
target_include_directories(autophage_common INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ==============================================================================
# Dependencies (Local)
# ==============================================================================

# spdlog
set(SPDLOG_INSTALL ON CACHE INTERNAL "")
add_subdirectory(third_party/spdlog)

# Catch2
if(AUTOPHAGE_BUILD_TESTS)
    add_subdirectory(third_party/Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
endif()

# nanobench
if(AUTOPHAGE_BUILD_BENCHMARKS)
    add_subdirectory(third_party/nanobench)
endif()

# SDL2
add_subdirectory(third_party/SDL)

# ==============================================================================
# Engine Modules
# ==============================================================================

# Core library (types, logging, memory, platform)
add_subdirectory(src/core)

# Window library (Phase 2)
add_subdirectory(src/window)

# Profiler library
add_subdirectory(src/profiler)

# ECS library (Phase 1)
add_subdirectory(src/ecs)

# Analyzer library (Phase 2)
# add_subdirectory(src/analyzer)

# Optimizer library (Phase 2)
# add_subdirectory(src/optimizer)

# Rewriter library (Phase 3)
# add_subdirectory(src/rewriter)

# ==============================================================================
# Main Engine Library
# ==============================================================================

add_library(autophage INTERFACE)
target_link_libraries(autophage INTERFACE
    autophage_core
    autophage_window
    autophage_profiler
    autophage_ecs
)

# ==============================================================================
# Tests
# ==============================================================================

if(AUTOPHAGE_BUILD_TESTS)
    enable_testing()
    include(CTest)
    include(Catch)
    add_subdirectory(tests)
endif()

# ==============================================================================
# Benchmarks
# ==============================================================================

if(AUTOPHAGE_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# ==============================================================================
# Examples
# ==============================================================================

if(AUTOPHAGE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ==============================================================================
# Installation
# ==============================================================================

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS autophage autophage_common autophage_warnings autophage_sanitizers
    EXPORT autophage-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/autophage
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT autophage-targets
    FILE autophage-targets.cmake
    NAMESPACE autophage::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/autophage
)

# ==============================================================================
# Summary
# ==============================================================================

message(STATUS "")
message(STATUS "=== Autophage Engine Configuration ===")
message(STATUS "Version:        ${PROJECT_VERSION}")
message(STATUS "Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler:       ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "")
message(STATUS "Options:")
message(STATUS "  Tests:        ${AUTOPHAGE_BUILD_TESTS}")
message(STATUS "  Benchmarks:   ${AUTOPHAGE_BUILD_BENCHMARKS}")
message(STATUS "  Examples:     ${AUTOPHAGE_BUILD_EXAMPLES}")
message(STATUS "  ASAN:         ${AUTOPHAGE_ENABLE_ASAN}")
message(STATUS "  UBSAN:        ${AUTOPHAGE_ENABLE_UBSAN}")
message(STATUS "  LLVM JIT:     ${AUTOPHAGE_USE_LLVM_JIT}")
message(STATUS "========================================")
message(STATUS "")
