# ==============================================================================
# Autophage Engine - Benchmarks
# ==============================================================================

add_executable(autophage_benchmarks
    bench_memory.cpp
    bench_profiler.cpp
)

target_link_libraries(autophage_benchmarks
    PRIVATE
        autophage
        nanobench
)

# Copy benchmark results to output directory
add_custom_command(TARGET autophage_benchmarks POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/benchmark-results
)
