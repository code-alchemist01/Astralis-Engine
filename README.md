# Astralis Engine

A 3D solar system simulation built with C++ and OpenGL. Explore randomly generated star systems with planets, moons, asteroid belts, and planetary rings.

## Features

- **Dynamic Solar Systems**: Each system is procedurally generated with unique characteristics
- **Realistic Sun**: Animated surface with solar flares and temperature-based coloring
- **Diverse Planets**: Various planet types with different sizes, colors, and compositions
- **Moon Systems**: Planets can have multiple moons orbiting around them
- **Asteroid Belts**: Scattered asteroid fields between planetary orbits
- **Planetary Rings**: Some planets feature beautiful ring systems
- **Free Camera**: Navigate through space with smooth camera controls
- **Particle Systems**: Dynamic particle effects for enhanced visual appeal
- **Procedural Generation**: Seed-based generation ensures reproducible yet varied results
- **ImGui Interface**: Real-time parameter adjustment and system information
- **Configurable Starfield**: Adjustable star density and brightness

## Controls

- **WASD**: Move camera forward/backward/left/right
- **Mouse**: Look around
- **Scroll**: Zoom in/out
- **ESC**: Exit application

## Building

### Prerequisites
- **CMake 3.21+** (updated requirement)
- **C++20 compatible compiler** (MSVC 2019+, GCC 10+, or Clang 12+)
- **OpenGL 3.3+ support**
- **Git** (for dependency fetching)

### Dependencies
The project automatically downloads and builds the following dependencies via CMake FetchContent:
- **GLFW 3.4** - Window management and input handling
- **GLM 1.0.1** - Mathematics library for graphics
- **ImGui 1.90.1** - Immediate mode GUI
- **spdlog 1.12.0** - Fast logging library
- **FastNoiseLite 1.1.1** - Procedural noise generation

### Build Steps
```bash
# Clone the repository
git clone <repository-url>
cd Astralis-Engine

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build . --config Release
```

### Running
```bash
# Windows
./build/Release/procedural_universe.exe

# Linux/macOS
./build/procedural_universe
```

### Command Line Options
- `--seed <number>` - Set the random seed for system generation
- `--planets <number>` - Set the number of planets (default: 8)

## Technical Details

### Architecture
The engine is built using modern C++20 with a modular architecture:

**Core Components:**
- **App**: Main application class managing the game loop
- **Window**: GLFW-based window management
- **Camera**: Free-flying camera with smooth controls
- **Shader**: OpenGL shader management and compilation
- **Texture**: Texture loading and management

**Rendering System:**
- **Geometry**: Mesh generation and management
- **ParticleSystem**: GPU-based particle rendering
- **Multiple specialized shaders** for different object types

**Solar System Simulation:**
- **SolarSystemManager**: Manages multiple solar systems
- **PlanetManager**: Handles planet generation and rendering
- **Planet, Moon, Sun**: Individual celestial body classes
- **AsteroidBelt, PlanetaryRings**: Special effect systems

### Technologies Used
- **OpenGL 3.3+** for rendering
- **GLFW** for window management and input
- **GLM** for mathematics operations
- **ImGui** for real-time interface elements
- **FastNoiseLite** for procedural noise generation
- **spdlog** for efficient logging
- **STB Image** for texture loading

### Procedural Generation
Each solar system is generated using seed-based algorithms, ensuring reproducible yet varied results. The generation includes:
- Planet positions, sizes, and orbital parameters
- Surface textures and atmospheric effects
- Moon systems with realistic orbital mechanics
- Asteroid belt distributions
- Planetary ring systems

## Project Structure
```
Astralis-Engine/
├── src/
│   ├── core/           # Core engine components
│   │   ├── App.*       # Main application
│   │   ├── Camera.*    # Camera system
│   │   ├── Shader.*    # Shader management
│   │   └── ...         # Other core systems
│   └── main.cpp        # Entry point
├── assets/
│   ├── shaders/        # GLSL shader files
│   └── textures/       # Texture assets
├── extern/             # External dependencies
└── CMakeLists.txt      # Build configuration
```

## Screenshots

<img width="1917" height="1003" alt="Ekran görüntüsü 2025-09-28 025702" src="https://github.com/user-attachments/assets/c0ebe88f-36f1-448e-a058-a11d093a3cc5" />

<img width="1901" height="1016" alt="Ekran görüntüsü 2025-09-28 025745" src="https://github.com/user-attachments/assets/145dd6ef-ec47-4cd5-821d-af4db009120c" />

<img width="1919" height="1018" alt="Ekran görüntüsü 2025-09-28 123458" src="https://github.com/user-attachments/assets/600a12c3-2dad-40b3-9163-10a0e2befded" />



## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

### Development Guidelines
- Follow the existing code style and conventions
- Add comments for complex algorithms
- Test your changes thoroughly
- Update documentation as needed

## License

This project is open source. Please check the repository for license information.

## Future Enhancements

- **Physics Simulation**: Realistic orbital mechanics and gravitational effects
- **Multiple Star Systems**: Binary and trinary star configurations
- **Enhanced Lighting**: Volumetric lighting and atmospheric scattering
- **Sound System**: Ambient space sounds and music
- **Save/Load System**: Ability to save and revisit interesting systems
- **VR Support**: Virtual reality exploration capabilities


