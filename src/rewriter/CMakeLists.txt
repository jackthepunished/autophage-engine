# src/rewriter/CMakeLists.txt

set(REWRITER_SOURCES
    jit_compiler.cpp
    hot_swap_manager.cpp
    rewriter.cpp
)

add_library(autophage_rewriter STATIC ${REWRITER_SOURCES})

target_link_libraries(autophage_rewriter
    PUBLIC
        autophage_common
        autophage_core
        autophage_ecs
)

if(AUTOPHAGE_USE_LLVM_JIT)
    find_package(LLVM REQUIRED CONFIG)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

    include_directories(${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})
    
    llvm_map_components_to_libnames(llvm_libs support core orcjit native)
    target_link_libraries(autophage_rewriter PRIVATE ${llvm_libs})
    target_compile_definitions(autophage_rewriter PRIVATE AUTOPHAGE_JIT_ENABLED)
endif()

install(TARGETS autophage_rewriter
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
