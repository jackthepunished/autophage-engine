# ==============================================================================
# Autophage Engine - ECS Module
# ==============================================================================

add_library(autophage_ecs STATIC
    ecs.cpp
)

target_link_libraries(autophage_ecs
    PUBLIC
        autophage_common
        autophage_core
)

target_include_directories(autophage_ecs
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

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
