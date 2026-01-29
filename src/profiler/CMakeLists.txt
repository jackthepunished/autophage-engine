# ==============================================================================
# Autophage Engine - Profiler Module
# ==============================================================================

add_library(autophage_profiler STATIC
    profiler.cpp
    scoped_timer.cpp
)

target_link_libraries(autophage_profiler
    PUBLIC
        autophage_core
)

target_include_directories(autophage_profiler
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Installation
install(TARGETS autophage_profiler
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
