# ==============================================================================
# Autophage Engine - Tests
# ==============================================================================

# Core module tests
add_executable(autophage_tests_core
    core/test_types.cpp
    core/test_memory.cpp
    core/test_logger.cpp
)

target_link_libraries(autophage_tests_core
    PRIVATE
        autophage_core
        Catch2::Catch2WithMain
)

catch_discover_tests(autophage_tests_core)

# Profiler module tests
add_executable(autophage_tests_profiler
    profiler/test_profiler.cpp
    profiler/test_scoped_timer.cpp
)

target_link_libraries(autophage_tests_profiler
    PRIVATE
        autophage_profiler
        Catch2::Catch2WithMain
)

catch_discover_tests(autophage_tests_profiler)

# ECS module tests
add_executable(autophage_tests_ecs
    ecs/test_entity.cpp
    ecs/test_component.cpp
    ecs/test_world.cpp
    ecs/test_system.cpp
)

target_link_libraries(autophage_tests_ecs
    PRIVATE
        autophage_ecs
        Catch2::Catch2WithMain
)

catch_discover_tests(autophage_tests_ecs)

# Combined test target
add_custom_target(autophage_tests
    DEPENDS
        autophage_tests_core
        autophage_tests_profiler
        autophage_tests_ecs
)
