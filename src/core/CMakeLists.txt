# ==============================================================================
# Autophage Engine - Core Module
# ==============================================================================

add_library(autophage_core STATIC
    # Platform & Types
    platform.cpp
    
    # Logging
    logger.cpp
    
    # Memory
    memory.cpp
)

target_link_libraries(autophage_core
    PUBLIC
        autophage_common
        spdlog::spdlog
)

target_include_directories(autophage_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Precompiled header for faster builds
target_precompile_headers(autophage_core
    PRIVATE
        <cstdint>
        <cstddef>
        <string>
        <string_view>
        <vector>
        <array>
        <memory>
        <utility>
        <type_traits>
        <concepts>
        <chrono>
        <functional>
        <optional>
        <expected>
        <span>
        <source_location>
)

# Installation
install(TARGETS autophage_core
    EXPORT autophage-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
