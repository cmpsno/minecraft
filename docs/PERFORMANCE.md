# Performance baseline

No reproducible runtime profile or benchmark has been recorded yet. The Phase 2 changes prepare CPU mesh generation to be independent from OpenGL uploads; this alone is not a performance claim.

## Current status

- Single-config CMake builds now default to `Release` when `CMAKE_BUILD_TYPE` is unset.
- The full CTest suite has 34 tests and passes with the matching MinGW runtime selected on `PATH`.
- Runtime optimization work should wait for profiling data and a reproducible benchmark.
