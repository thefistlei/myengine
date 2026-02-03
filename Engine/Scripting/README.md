# Lua Scripting Module

This module provides Lua scripting support for the MyEngine game engine. Scripts can be attached to entities to control their behavior at runtime.

## Features

- **Lua 5.4 Integration**: Full Lua scripting support with custom memory management
- **Entity Component System**: Scripts can access and modify entity components
- **Vec3 Math Library**: Complete vector math operations available in Lua
- **Lifecycle Callbacks**: OnInit, OnUpdate, OnDestroy functions
- **Hot Reload Support**: Scripts can be reloaded at runtime (future feature)
- **Memory Management**: Custom allocator with configurable limits

## Architecture

### Core Components

1. **LuaVM** - Singleton managing the Lua virtual machine
   - Memory tracking and GC configuration
   - Error handling and reporting
   - Script search paths

2. **ScriptComponent** - ECS component for attaching scripts to entities
   - Script lifecycle management
   - Environment isolation per script instance
   - Function caching for performance

3. **ScriptSystem** - System that updates all active scripts
   - Loads and initializes scripts
   - Calls OnUpdate every frame
   - Manages script cleanup

4. **LuaBridge** - Bindings between C++ and Lua
   - Entity API (GetPosition, SetPosition, etc.)
   - Vec3 math operations
   - Extensible binding system

## Usage

### Installing Lua

Using vcpkg:
```bash
vcpkg install lua:x64-windows
```

### Creating a Script

Create a `.lua` file in `Runtime/Scripts/` or `Assets/Scripts/`:

```lua
-- MyScript.lua

local myVariable = 0

function OnInit()
    print("Script initialized!")
    local pos = self:GetPosition()
    print("Starting at: " .. tostring(pos))
end

function OnUpdate(deltaTime)
    myVariable = myVariable + deltaTime
    
    -- Move the entity
    local pos = self:GetPosition()
    pos.y = pos.y + deltaTime
    self:SetPosition(pos)
end

function OnDestroy()
    print("Script destroyed!")
end
```

### Attaching a Script to an Entity

```cpp
// In C++ code
Entity entity = registry.CreateEntity();
entity.AddComponent<ScriptComponent>("Runtime/Scripts/MyScript.lua");
```

### Available Lua API

#### Entity Methods

- `self:GetPosition()` - Returns Vec3
- `self:SetPosition(vec3)` - Sets position
- `self:GetRotation()` - Returns Vec3 (euler angles)
- `self:SetRotation(vec3)` - Sets rotation
- `self:GetScale()` - Returns Vec3
- `self:SetScale(vec3)` - Sets scale
- `self:GetTag()` - Returns string
- `self:SetTag(string)` - Sets tag

#### Vec3 Operations

```lua
-- Construction
local v = Vec3(1, 2, 3)

-- Component access
local x = v.x
local y = v.y
local z = v.z
v.x = 5.0

-- Math operations
local sum = v1 + v2      -- Addition
local diff = v1 - v2     -- Subtraction
local scaled = v * 2.0   -- Scalar multiplication

-- Methods
local length = v:Length()
local normalized = v:Normalize()
local dot = v1:Dot(v2)
local cross = v1:Cross(v2)

-- String conversion
print(tostring(v))  -- Vec3(1.00, 2.00, 3.00)
```

## Example Scripts

### 1. Rotator.lua
Continuously rotates an object around the Y axis.

### 2. Bouncer.lua
Makes an object bounce up and down using a sine wave.

### 3. Orbiter.lua
Orbits an object around a center point and demonstrates Vec3 math operations.

## Configuration

The Lua VM can be configured in `ScriptSystem::Init()`:

```cpp
LuaVMConfig config;
config.initialMemoryMB = 64;      // Initial memory allocation
config.maxMemoryMB = 512;         // Maximum memory limit
config.gcStepMultiplier = 200;    // GC aggressiveness
config.gcPause = 200;             // GC pause percentage
config.searchPaths = {            // Script search paths
    "Runtime/Scripts",
    "Assets/Scripts"
};
```

## Performance Considerations

1. **Function Caching**: Script functions (OnInit, OnUpdate, OnDestroy) are cached as Lua references for fast lookups
2. **Environment Isolation**: Each script instance has its own environment table to prevent variable conflicts
3. **Memory Tracking**: Custom allocator tracks memory usage and enforces limits
4. **Lazy Loading**: Scripts are loaded on-demand when first accessed

## Future Enhancements

- [ ] Hot reload support
- [ ] Script debugging integration
- [ ] Coroutine support for async operations
- [ ] More component bindings (Mesh, Material, Camera, etc.)
- [ ] Event system integration
- [ ] Script profiling and performance metrics
- [ ] Script editor panel in the engine

## Files

```
Engine/Scripting/
├── LuaVM.h/cpp              - Lua virtual machine manager
├── ScriptComponent.h/cpp    - ECS script component
├── ScriptSystem.h/cpp       - Script update system
├── LuaBridge.h/cpp          - C++/Lua bindings
└── CMakeLists.txt           - Build configuration

Runtime/Scripts/
├── Rotator.lua              - Example: rotation script
├── Bouncer.lua              - Example: bouncing motion
└── Orbiter.lua              - Example: orbital motion with Vec3 math
```

## Building

The Scripting module is automatically built as part of the engine. Make sure Lua is installed via vcpkg before building.

## License

Part of MyEngine project.
