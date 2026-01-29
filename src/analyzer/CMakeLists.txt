# ==============================================================================
# Autophage Engine - Analyzer Module
# ==============================================================================

add_library(autophage_analyzer STATIC
    stats_collector.cpp
    access_pattern_tracker.cpp
)

target_link_libraries(autophage_analyzer
    PUBLIC
        autophage_core
        autophage_profiler
)

target_include_directories(autophage_analyzer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Installation
install(TARGETS autophage_analyzer
    EXPORT autophage-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
