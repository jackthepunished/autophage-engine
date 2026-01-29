add_library(autophage_optimizer STATIC
    optimizer.cpp
)

target_link_libraries(autophage_optimizer
    PUBLIC
        autophage_core
        autophage_analyzer
        autophage_ecs
)

target_include_directories(autophage_optimizer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

install(TARGETS autophage_optimizer
    EXPORT autophage-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
