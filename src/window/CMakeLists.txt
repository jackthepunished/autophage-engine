add_library(autophage_window
    window_sdl.cpp
)

target_link_libraries(autophage_window
    PUBLIC
        autophage_core
    PRIVATE
        SDL2::SDL2
)

target_include_directories(autophage_window PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(MSVC)
    target_compile_options(autophage_window PRIVATE /wd4251) # Suppress DLL export warning for SDL
endif()

install(TARGETS autophage_window
    EXPORT autophage-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
