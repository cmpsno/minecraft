# Refactor findings

## Build structure after Phase 2

- `MinecraftLogic` contains world/player/core logic and links publicly only to GLM. `MinecraftRender` contains the rendering code and links to `MinecraftLogic`, glad, and GLFW. `MinecraftLib` contains the application layer and links to `MinecraftRender`.
- The test executables now link to the narrowest appropriate library target. Pure-logic tests do not link glad or GLFW.
- `src/world/Chunk` stores CPU-side `ChunkMeshData`. `World` provides completed mesh data for the renderer; GPU meshes are owned by `ChunkRenderer`/`GpuMesh` under `src/renderer`.
- `src/world` and `src/player` contain no glad, GLFW, `glBind`, or `glGen` references.

## Build and test baseline triage

- **Baseline SHA:** `443ea36` (`Merge pull request #48 from isaiahcampusano/isaiahcampusano-minecraft-pathfinding`), before Phase 1 and Phase 2.
- **Toolchain:** Ninja with MinGW-W64 x86_64 UCRT POSIX/SEH GCC 16.1.0 from WinLibs.
- **Pre-Phase-1 results with the default `PATH`:** 27 passed, 6 failed, 0 skipped out of 33. The failures were `save_load`, `furnace_persistence`, `furnace_render`, `cooking_application`, `menu_core`, and `menus_application`; each exited with `0xc0000139` (`STATUS_ENTRYPOINT_NOT_FOUND`).
- **Pre-Phase-1 results with the matching compiler runtime directory first on `PATH`:** all 33 tests passed.
- **Post-Phase-1 results with the default `PATH`:** the same 27 passed and the same 6 loader failures occurred.
- **Root cause:** `C:\Program Files\Git\mingw64\bin` preceded the configured compiler on `PATH`. Git for Windows' `libgcc_s_seh-1.dll` shadowed the matching MinGW runtime DLL. Direct execution failed with the default `PATH` and succeeded when the configured toolchain's `bin` directory was prepended. An isolation check established that selecting the toolchain's `libgcc_s_seh-1.dll` fixed the failure.
- **Resolution:** no source or linker change was needed. Put the matching compiler `bin` directory first on `PATH` when running MinGW-built executables.
- **Phase 2 validation:** all 34 tests, including `chunk_mesh_golden`, passed with the matching MinGW runtime first on `PATH`.

The original baseline comparison used 33 tests; Phase 2A added the mesh golden test, bringing the current suite to 34.
