# PlanetGen

A small, modular C++ planet generator and interactive viewer. The current renderer is a dependency-light CPU/X11 viewer so it works without a game engine or GPU API setup, but the generator is separated from the renderer so a Vulkan/OpenGL/compute backend can be added later.

## Features

- Rotatable shaded planet sphere.
- Procedural continent masks and terrain elevation driven by reusable 3D value-noise modules.
- Ocean layer that can be toggled and raised/lowered at runtime.
- Generator code is isolated in `planetgen_core` so you can replace or stack terrain modules without touching the viewer.

## Build

```bash
cmake -S . -B build
cmake --build build
```

The viewer uses X11. On Linux you need X11 development headers/libraries installed. To build just the generator library:

```bash
cmake -S . -B build -DPLANETGEN_BUILD_VIEWER=OFF
cmake --build build
```

## Run

```bash
./build/planetgen_viewer
```

Controls:

| Input | Action |
| --- | --- |
| Left mouse drag | Rotate the planet |
| `O` | Toggle ocean visibility |
| `+` / `=` | Raise ocean height |
| `-` / `_` | Lower ocean height |
| `[` / `]` | Decrease/increase terrain exaggeration |
| `R` | Reset camera and ocean height |
| `Q` / `Esc` | Quit |

## Extending terrain

Start with `planetgen::ITerrainModule` in `include/planetgen/Terrain.hpp`. Add another implementation that returns a `TerrainSample`, then plug it into `planetgen::Planet` or compose it with `ContinentsTerrain`. The viewer only asks the planet for samples on a unit direction, so the same generator can feed a mesh, a GPU texture, or this CPU renderer.
