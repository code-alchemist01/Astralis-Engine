# Contributing to Astralis Engine

Thank you for your interest in contributing to Astralis Engine! This document provides guidelines for contributing to our 3D solar system simulation project.

## Development Setup

### Prerequisites
- CMake 3.21+
- C++20 compatible compiler (MSVC 2019+, GCC 10+, or Clang 12+)
- OpenGL 3.3+ support
- Git

### Getting Started
1. Fork the repository
2. Clone your fork: `git clone https://github.com/yourusername/Astralis-Engine.git`
3. Create a feature branch: `git checkout -b feature/amazing-feature`
4. Follow the build instructions in README.md

### Building the Project
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Code Style Guidelines

### C++ Standards
- Use C++20 features where appropriate
- Follow RAII principles
- Use smart pointers instead of raw pointers
- Prefer const-correctness

### Naming Conventions
- Classes: `PascalCase` (e.g., `SolarSystemManager`)
- Functions/Methods: `camelCase` (e.g., `generateSolarSystem`)
- Variables: `camelCase` with trailing underscore for members (e.g., `systemSeed_`)
- Constants: `UPPER_SNAKE_CASE`

### File Organization
- Header files: `.hpp`
- Implementation files: `.cpp`
- Keep headers in the same directory as implementation
- Use include guards or `#pragma once`

## Project Structure
- `src/core/`: Core engine components
- `assets/shaders/`: GLSL shader files
- `assets/textures/`: Texture assets
- `extern/`: External dependencies

## Types of Contributions

### Bug Fixes
- Check existing issues before creating new ones
- Include reproduction steps and system information
- Test your fix thoroughly

### New Features
- Discuss major features in an issue first
- Consider performance implications
- Update documentation as needed

### Performance Improvements
- Profile your changes
- Include benchmarks if possible
- Consider memory usage impact

## Submitting Changes

1. Ensure your code builds without warnings
2. Test your changes on your target platform
3. Update documentation if needed
4. Write clear commit messages
5. Create a pull request with a detailed description

## Reporting Issues

Please use the issue templates and include:
- System information (OS, GPU, OpenGL version)
- Steps to reproduce
- Expected vs actual behavior
- Screenshots for visual issues
- Console output for crashes

## Questions?

Feel free to open an issue for questions about contributing!