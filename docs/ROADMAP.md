# Autophage Engine Roadmap

> Self-Rewriting, Runtime-Adaptive Game Engine

---

## Overview

```
Phase 0: Foundation     [Q1 2025]  ████████░░ Infrastructure
Phase 1: Core Systems   [Q2 2025]  ░░░░░░░░░░ Profiler + ECS
Phase 2: Intelligence   [Q3 2025]  ░░░░░░░░░░ Analyzer + Optimizer
Phase 3: Rewriting      [Q4 2025]  ░░░░░░░░░░ Hot-Swap + JIT
Phase 4: Polish         [Q1 2026]  ░░░░░░░░░░ Tools + Demo
```

---

## Phase 0: Foundation (Complete)

> Build infrastructure and project skeleton

### 0.1 Project Setup
- [x] Technical documentation
- [x] Agent and skill definitions
- [x] CMake build system (C++23, multi-platform)
- [x] Folder structure and module boundaries
- [x] CI/CD pipeline (GitHub Actions)
- [x] Code style (clang-format, clang-tidy)
- [x] Testing framework (Catch2)
- [x] Benchmarking framework (nanobench)

### 0.2 Core Utilities
- [x] Logging system (spdlog integration)
- [x] Assert and error handling macros
- [x] Memory utilities (aligned allocation, pool allocator base)
- [x] Type ID system (compile-time hashing)
- [x] Platform abstraction layer

### Deliverables
- Compiling empty project on Windows/Linux
- CI passing with lint + build + test
- Basic logging and memory utilities

---

## Phase 1: Core Systems

> Runtime Profiler + Entity Component System

### 1.1 Runtime Profiler
- [x] Scoped timer (zones)
- [x] Frame time statistics (avg, p95, spikes)
- [x] Memory allocation tracking (basic)
- [ ] Double-buffered metric collection
- [ ] Platform backends:
  - [ ] Windows (QueryPerformanceCounter, ETW)
  - [ ] Linux (perf_event)
  - [x] RDTSC-based cycles (basic)
- [ ] GPU timing queries (placeholder/Vulkan)

### 1.2 ECS Foundation
- [x] Entity ID management (generational indices)
- [ ] Component storage (archetype-based)
- [x] AoS storage implementation (sparse set)
- [ ] SoA storage implementation
- [ ] Runtime layout conversion (AoS ↔ SoA)
- [x] System registration and execution
- [x] Query system (component iteration)

### 1.3 Basic Systems
- [ ] Transform system (position, rotation, scale)
- [x] Hierarchy system (parent-child)
- [x] Velocity system (simple physics)

### Deliverables
- Profiler capturing frame metrics
- ECS with 10K+ entities at 60fps
- Benchmark: AoS vs SoA performance comparison

---

## Phase 2: Intelligence

> Analyzer + Rule-Based Optimizer

### 2.1 Analyzer
- [ ] Profiler data aggregation
- [ ] Hot path detection algorithm
- [ ] ECS execution graph builder
- [ ] System dependency analysis
- [ ] Memory access pattern detection
- [ ] Optimization candidate scoring
- [ ] Confidence score calculation

### 2.2 Rule-Based Optimizer
- [ ] Rule definition DSL/format
- [ ] Built-in rules:
  - [ ] Entity count threshold → SIMD variant
  - [ ] Cache miss ratio → SoA rewrite
  - [ ] GPU idle time → compute offload
- [ ] Rule evaluation engine
- [ ] Decision logging and audit trail

### 2.3 System Variants
- [ ] Variant registration API
- [ ] Scalar implementation (baseline)
- [ ] SIMD implementation (SSE/AVX)
- [ ] Variant selection at runtime
- [ ] A/B comparison infrastructure

### Deliverables
- Analyzer identifying slow systems
- Optimizer suggesting improvements
- Auto-switching between system variants

---

## Phase 3: Rewriting

> Hot-Swap + LLVM JIT + Validation

### 3.1 Hot-Swap Infrastructure
- [ ] Function pointer redirection
- [ ] DLL/SO hot reload (Windows/Linux)
- [ ] State preservation across swaps
- [ ] Thread synchronization during swap

### 3.2 LLVM JIT Integration
- [ ] LLVM/ORC JIT setup
- [ ] IR generation for kernels
- [ ] Runtime compilation pipeline
- [ ] Optimized kernel caching
- [ ] Fallback to precompiled code

### 3.3 Safety & Rollback
- [ ] Deterministic replay window
- [ ] Pre-swap state snapshot
- [ ] Performance regression detection
- [ ] Memory correctness validation
- [ ] Automatic rollback on failure
- [ ] Strategy blacklisting

### 3.4 Rewrite Targets
- [ ] Transform system hot-swap
- [ ] Physics integration swap
- [ ] Memory layout migration (live)

### Deliverables
- Live system replacement without restart
- JIT-compiled kernels running in-engine
- Automatic rollback on regression

---

## Phase 4: Polish & Demo

> Developer Tools + Showcase

### 4.1 Visual Profiler UI
- [ ] ImGui integration
- [ ] Timeline view (systems & threads)
- [ ] Memory heatmaps
- [ ] Before/after comparison
- [ ] Live optimization log

### 4.2 Debug Console
- [ ] Command parser
- [ ] Manual override commands
- [ ] Rewrite freeze/force modes
- [ ] Metric queries

### 4.3 Demo Application
- [ ] Particle system (100K+ particles)
- [ ] Dynamic entity spawning
- [ ] Stress test scenarios
- [ ] Self-optimization showcase

### 4.4 Documentation
- [ ] API reference (Doxygen)
- [ ] Architecture guide
- [ ] Tutorial: Adding custom systems
- [ ] Tutorial: Writing optimization rules

### Deliverables
- Interactive profiler UI
- Demo showing live self-optimization
- Complete documentation

---

## Future Work (Post v1.0)

| Feature | Description |
|---------|-------------|
| Cross-frame prediction | Predict workload changes |
| Multi-process rewriting | Parallel JIT compilation |
| Networked determinism | Distributed replay |
| AI-assisted optimization | LLM decision support |
| GPU compute kernels | CUDA/Vulkan compute |
| WASM replay viewer | Web-based visualization |

---

## Success Metrics

| Metric | Target |
|--------|--------|
| Frame time stability | < 5% variance |
| Hot-swap latency | < 1 frame |
| JIT compilation | < 100ms for kernel |
| Rollback time | < 2 frames |
| Profiler overhead | < 1% |

---

## Tech Stack

| Component | Technology |
|-----------|------------|
| Language | C++23 (MSVC, Clang, GCC) |
| Build | CMake 3.20+ |
| JIT | LLVM 17+ (ORC JIT) |
| Testing | Catch2 |
| Benchmark | nanobench |
| Logging | spdlog |
| UI | Dear ImGui |
| Platforms | Windows, Linux (macOS experimental) |

---

## Contributing

See `CONTRIBUTING.md` for guidelines.

Priority areas for contribution:
1. Platform backends (profiler)
2. SIMD system implementations
3. Optimization rules
4. Documentation and examples
