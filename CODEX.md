# Minecraft v1

## Goal

Build a finite 1000×1000 Minecraft-style voxel world in C++17 and OpenGL. The terrain is a seeded fractal-noise heightmap with rolling hills: Bedrock at y=0, stone below, a dirt band, and a Grass, Sand, or Gravel surface, with sparse full oak trees rooted at the local surface height. 3D-noise carvers wind cave tunnels through the stone, seeded with coal and iron ore veins.

## Architecture

- GLFW owns the window and input.
- GLAD loads OpenGL 3.3 Core functions.
- GLM supplies vector and matrix math.
- `World` streams chunks around the player and enforces the finite boundary.
- `Chunk` stores 16×256×16 blocks and emits only faces adjacent to air.
- `Renderer`, `Mesh`, `Shader`, and `Texture` own OpenGL resources.
- `Camera` and `Raycaster` provide first-person controls and editing.

The atlas is generated in memory to keep the executable self-contained. Loaded chunks are bounded by render distance; all 3,969 possible chunks are never resident at once.

## v1 status

- [x] CMake dependency setup
- [x] Window and input
- [x] Shader pipeline and texture atlas
- [x] First-person camera
- [x] Noise terrain generation (hills, caves, ores)
- [x] 1000×1000 boundary
- [x] Chunk streaming and visible-face meshing
- [x] Block breaking and placing
- [x] Basic player collision

