# ==============================================================================
# Autophage Engine - ECS Module
# ==============================================================================

add_library(autophage_ecs STATIC
    ecs.cpp
    systems/render_system.cpp
)

target_link_libraries(autophage_ecs
    PUBLIC
        autophage_common
        autophage_core
        autophage_window
)

target_include_directories(autophage_ecs
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_executable(autophage_ecs_debug ecs_debug.cpp)
target_link_libraries(autophage_ecs_debug PRIVATE autophage_ecs)

# Enable SIMD for optimized systems
if(MSVC)
    target_compile_options(autophage_ecs PRIVATE /arch:AVX2)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(autophage_ecs PRIVATE -mavx2 -mfma)
endif()

set_target_properties(autophage_ecs PROPERTIES
    OUTPUT_NAME "autophage_ecs"
)

# Installation
install(TARGETS autophage_ecs
    EXPORT autophage-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
