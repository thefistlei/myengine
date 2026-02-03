# MyEngine - 3D Game Engine

A learning project focused on understanding modern 3D engine architecture, rendering systems, and gameplay frameworks.

## Overview

This is a real-time 3D engine built from scratch to explore:
- Modular rendering pipeline with multiple passes (lighting, water, terrain, grass)
- Entity Component System (ECS) architecture
- Editor tools and workflow
- Lua scripting integration
- Modern C++20 features

## Features

### Rendering System
- **Multi-Pass Rendering Pipeline**
  - Geometry Pass
  - Sky Pass
  - Water Pass with reflections
  - Terrain Pass
  - Grass Pass
  - Skeletal Animation Pass
  - Particle Pass
  - Post-Processing Pass

### Editor
- **Unity-style Editor Interface**
  - Scene Hierarchy Panel
  - Properties/Inspector Panel
  - Viewport with 3D scene rendering
  - Asset Browser with drag-and-drop
  - Play Mode (Editor ↔ Game mode switching)

### ECS System
- Component-based entity management
- Transform hierarchy with parent-child relationships
- Efficient component storage and queries

### Scripting
- **Lua Integration** (optional)
  - Entity lifecycle callbacks (OnInit, OnUpdate, OnDestroy)
  - Vec3 math operations
  - Transform manipulation
  - Hot-reload support (planned)

### Asset Management
- FBX model loading (via Assimp)
- Texture loading
- Material system
- Asset database with GUID tracking

## Architecture

```
MyEngine/
├── Engine/
│   ├── Core/          # Application, logging, memory, events
│   ├── Platform/      # Window, input, file system
│   ├── Math/          # Vec3, Mat4, Quat, Transform
│   ├── Rendering/     # Renderer, passes, shaders, buffers
│   ├── ECS/           # Entity, components, registry
│   ├── Scene/         # Scene graph
│   ├── Editor/        # Editor layer and panels
│   ├── Animation/     # Skeletal animation
│   ├── Particle/      # Particle systems
│   ├── Scripting/     # Lua VM, bindings, script components
│   └── Resource/      # Asset loading and management
├── Runtime/
│   ├── Scripts/       # Lua example scripts
│   └── main.cpp       # Application entry point
└── ThirdParty/        # External dependencies
```

## Building

### Prerequisites
- **CMake** 3.20+
- **C++20** compatible compiler (MSVC, GCC, Clang)
- **vcpkg** (for dependencies)

### Dependencies
- **GLFW** - Window and input
- **Glad** - OpenGL loader
- **GLM** - Math library
- **ImGui** - Editor UI
- **spdlog** - Logging
- **Assimp** - 3D model loading (optional)
- **Lua** - Scripting (optional, x64 version required)

### Build Instructions

1. **Install dependencies via vcpkg:**
   ```bash
   vcpkg install glfw3:x64-windows
   vcpkg install glm:x64-windows
   vcpkg install assimp:x64-windows
   vcpkg install lua:x64-windows  # Optional for scripting
   ```

2. **Configure CMake:**
   ```bash
   cmake -B build-root -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
   ```

3. **Build:**
   ```bash
   cmake --build build-root
   ```

4. **Run:**
   ```bash
   ./build-root/bin/Debug/MyEngine.exe
   ```

## Development Challenges & Learnings

### Challenges Encountered
- **Modular Renderer**: Keeping the renderer modular as lighting, water, terrain and grass passes accumulated
- **ECS Design**: Balancing flexibility with simplicity in the Entity Component System
- **Render Order**: Managing render pass dependencies and execution order
- **Camera/Math Issues**: Debugging view-projection problems that only appeared from certain angles
- **Editor Complexity**: Realizing that tooling and editor features require as much effort as the rendering itself

### Key Learnings
- **Debug Visualizations**: Built debug overlays for lighting and render passes - invaluable for troubleshooting
- **Early Decoupling**: Forcing system decoupling from the start prevented major refactoring pain later
- **Iterative Development**: Working with small test scenes instead of complex ones accelerated debugging
- **AI-Assisted Development**: Using AI tools (Qoder) helped refactor messy systems and clarified rendering concepts

## Usage Examples

### Creating Entities with Scripts

```cpp
// C++ side
Entity entity = registry.CreateEntity();
entity.AddComponent<TagComponent>("Player");
entity.AddComponent<TransformComponent>(Vec3(0, 5, 0));
entity.AddComponent<ScriptComponent>("Runtime/Scripts/Player.lua");
```

```lua
-- Lua side (Player.lua)
local speed = 10.0

function OnInit()
    print("Player initialized at: " .. tostring(self:GetPosition()))
end

function OnUpdate(deltaTime)
    local pos = self:GetPosition()
    pos.y = pos.y + speed * deltaTime
    self:SetPosition(pos)
end
```

### Editor Workflow

1. **Scene Setup**: Create entities in the Scene Hierarchy
2. **Component Configuration**: Adjust properties in the Inspector
3. **Asset Management**: Drag models from Asset Browser to Viewport
4. **Play Mode**: Press ▶ Play to test, ■ Stop to return to editing

## Project Status

### ✅ Completed
- Core engine architecture
- Multi-pass rendering pipeline
- ECS system with transform hierarchy
- Unity-style editor interface
- Asset browser with drag-and-drop
- FBX model loading
- Lua scripting module (requires x64 Lua)
- Play/Stop mode switching

### 🚧 In Progress
- Material editor
- Lighting system improvements
- Physics integration

### 📋 Planned
- Scene serialization
- Prefab system
- Audio system
- Script hot-reloading
- Build pipeline for standalone games

## Notes

### Lua Scripting
The engine supports Lua scripting, but requires an **x64 (64-bit)** version of Lua. If CMake detects an x86 (32-bit) Lua library, scripting will be automatically disabled to allow the build to succeed.

To enable Lua scripting:
- Install x64 Lua via vcpkg: `vcpkg install lua:x64-windows`
- Or download x64 binaries from [LuaBinaries](https://luabinaries.sourceforge.net/)

### Editor Controls
- **Right Mouse**: Rotate camera
- **Middle Mouse**: Pan camera
- **Mouse Wheel**: Zoom camera
- **W/A/S/D**: Move camera forward/left/backward/right
- **Q/E**: Move camera down/up

## License

This is a learning project. Feel free to use and modify as you see fit.

## Acknowledgments

- Built with assistance from **Qoder AI** for refactoring and architectural guidance
- Inspired by Unity and Unreal Engine editor workflows
- Uses multiple open-source libraries (see Dependencies section)
