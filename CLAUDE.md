# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

QuickNES_Core is a libretro core implementation of the QuickNES NES emulator. It provides high-speed, accurate Nintendo Entertainment System emulation as a shared library that can be used by RetroArch and other libretro frontends.

## Build Commands

### Standard Build (Linux/macOS)
```bash
make
```

### Cross-platform builds
```bash
# Windows (MinGW)
make platform=win

# Android (requires Android NDK)
cd jni && ndk-build

# iOS
make platform=ios-arm64

# Various console platforms
make platform=ps2       # PlayStation 2
make platform=psp1      # PlayStation Portable
make platform=vita      # PlayStation Vita
make platform=ctr       # Nintendo 3DS
make platform=ngc       # Nintendo GameCube
make platform=wii       # Nintendo Wii
make platform=wiiu      # Nintendo WiiU
make platform=libnx     # Nintendo Switch
```

### Debug Build
```bash
make DEBUG=1
```

### Clean Build
```bash
make clean          # Remove objects and target
make clean-objs     # Remove only object files
```

## Core Architecture

### Main Components

- **libretro/libretro.cpp**: Main libretro API implementation and interface
- **nes_emu/**: Core NES emulation engine containing:
  - **Nes_Emu**: High-level emulator interface (nes_emu/Nes_Emu.h:21)
  - **Nes_Core**: Internal NES system implementation (nes_emu/Nes_Core.h:17)
  - **Nes_Cpu**: 6502 CPU emulation
  - **Nes_Ppu**: Picture Processing Unit (graphics)
  - **Nes_Apu**: Audio Processing Unit (sound)
  - **Nes_Cart**: Cartridge handling and ROM loading
  - **Nes_Mapper**: Memory mapper system for different cartridge types

### Mapper System

The emulator uses a plugin-based mapper system to support different cartridge types:
- **nes_emu/mappers/**: Contains 50+ mapper implementations as header files
- **Nes_Mapper**: Base mapper class (nes_emu/Nes_Mapper.h:17)
- Mappers are registered via creator functions and instantiated dynamically

### Audio System

- **Blip_Buffer**: High-quality band-limited audio synthesis
- **Effects_Buffer**: Audio effects processing
- **Multi_Buffer**: Multiple audio channel management
- **Nes_Apu**: Main APU implementation with support for:
  - Pulse waves, triangle, noise, and DMC channels
  - External sound chips (FME7, Namco, VRC6, VRC7)

### Video System

- **Nes_Ppu**: PPU implementation with accurate timing
- **nes_ntsc**: NTSC video filter for authentic CRT appearance
- Rendering pipeline supports multiple pixel formats
- Image dimensions: 256x240 pixels (standard NES resolution)

## Key Files

- **Makefile**: Main build system with extensive platform support
- **Makefile.common**: Source file definitions and include paths
- **link.T**: Linker script for symbol visibility
- **libretro_core_options.h**: Core option definitions for frontend configuration
- **jni/**: Android NDK build configuration

## Development Notes

- The codebase follows C++ with some C compatibility
- Uses namespace-less design with class prefixes (Nes_*, Blip_*)
- Extensive platform support through conditional compilation
- State save/load functionality implemented throughout
- Memory management uses custom allocators in some areas
- Audio synthesis uses fixed-point arithmetic for performance

## CI/CD

The project uses both GitLab CI and Travis CI for automated builds:
- **GitLab CI**: Comprehensive multi-platform builds (50+ targets)
- **Travis CI**: Legacy builds for specific platforms
- Automated builds for desktop, mobile, and console platforms