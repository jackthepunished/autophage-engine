# Autophage Engine
**A Self-Rewriting, Runtime-Adaptive Game Engine**

> *When the workload changes, the engine changes itself.*

Autophage Engine is a research-grade, production-inspired game engine exploring the concept of **runtime self-optimization**. It treats performance as a dynamic property, using a built-in profiler and analyzer to detect bottlenecks and "rewrite" its own execution strategies (hot-swapping component layouts, system implementations, or switching to JIT-compiled kernels) while the game is running.

##  Project Status
**Current Phase:** Phase 1 (Core Systems)
> Phase 0 (foundation) is complete: build system, CI, logging, memory utilities, and core platform/types are in place. Phase 1 is in progress with a functional ECS and basic profiler, and ongoing work on deeper profiling and ECS performance.

##  Key Features
- **Runtime Profiling**: Deep visibility into CPU, memory, and frame timing.
- **Adaptive ECS**: Entity-Component-System architecture designed for hot-swappable storage and execution paths.
- **Self-Optimization loops** (Planned): Analyzers that trigger hot-reloading of optimized system variants (e.g., switching from scalar to SIMD based on entity count).
- **Safety & Rollback**: All rewrites are deterministic and can be rolled back if regressions are detected.

##  Getting Started

### Prerequisites
- **CMake**: 3.20 or newer
- **Compiler**: C++23 compliant compiler
    - Windows: MSVC (Visual Studio 2022 v17.6+)
    - Linux: Clang 16+ or GCC 13+
- **Git**

### Build Instructions

1.  **Clone the repository**
    ```bash
    git clone https://github.com/your-username/autophage-engine.git
    cd autophage-engine
    ```

2.  **Configure the project**
    ```bash
    cmake -B build
    ```

3.  **Build**
    ```bash
    cmake --build build --config Debug
    ```

4.  **Run Tests**
    ```bash
    ctest --test-dir build --output-on-failure
    ```

##  Roadmap

- [x] **Phase 0: Foundation** (Build system, Logging, Memory Allocators, Tests)
- [ ] **Phase 1: Core Systems** (Runtime Profiler, ECS Foundation)
- [ ] **Phase 2: Intelligence** (Analyzer, Rule-Based Optimizer)
- [ ] **Phase 3: Rewriting** (Hot-Swap Infrastructure, JIT)
- [ ] **Phase 4: Polish** (Visual Profiler UI, Demos)

##  Project Structure
- `src/core`: Low-level utilities (Memory, Logging, Asserts).
- `src/ecs`: Entity-Component-System implementation.
- `src/profiler`: Runtime metric collection.
- `tests/`: Catch2-based unit tests.
- `docs/`: Technical documentation and design notes.

##  License
This project is available under the MIT License. See `LICENSE` for details.
