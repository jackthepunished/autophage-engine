# ==============================================================================
# Autophage Engine - Examples
# ==============================================================================

add_executable(autophage_demo
    main.cpp
)

target_link_libraries(autophage_demo
    PRIVATE
        autophage
)
