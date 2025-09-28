# Procedural Universe Generator

A 3D solar system simulation built with C++ and OpenGL. Explore randomly generated star systems with planets, moons, asteroid belts, and planetary rings.

## Features

- **Dynamic Solar Systems**: Each system is procedurally generated with unique characteristics
- **Realistic Sun**: Animated surface with solar flares and temperature-based coloring
- **Diverse Planets**: Various planet types with different sizes, colors, and compositions
- **Moon Systems**: Planets can have multiple moons orbiting around them
- **Asteroid Belts**: Scattered asteroid fields between planetary orbits
- **Planetary Rings**: Some planets feature beautiful ring systems
- **Free Camera**: Navigate through space with smooth camera controls

## Controls

- **WASD**: Move camera forward/backward/left/right
- **Mouse**: Look around
- **Scroll**: Zoom in/out
- **ESC**: Exit application

## Building

### Requirements
- CMake 3.15+
- C++17 compatible compiler
- OpenGL 3.3+ support

### Build Steps
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Running
```bash
./build/Release/procedural_universe.exe
```

## Technical Details

Built using modern C++ with:
- OpenGL for rendering
- GLFW for window management
- GLM for mathematics
- ImGui for interface elements
- FastNoiseLite for procedural generation
- spdlog for logging

Each solar system is generated using seed-based algorithms, ensuring reproducible yet varied results.

## Screenshots

Navigate through space and discover unique planetary systems with realistic lighting and atmospheric effects.

---

*Developed as an exploration into procedural generation and 3D graphics programming.*