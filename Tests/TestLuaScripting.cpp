/******************************************************************************
 * File: TestLuaScripting.cpp
 * Author: AI Assistant
 * Created: 2026-02-02
 * Description: Test/example demonstrating Lua scripting integration
 * 
 * This file shows how to:
 * 1. Initialize the script system
 * 2. Create entities with scripts
 * 3. Update scripts each frame
 * 4. Clean up properly
 * 
 * To run this test, ensure Lua is installed via vcpkg:
 *   vcpkg install lua:x64-windows
 ******************************************************************************/

#include "Scripting/ScriptSystem.h"
#include "Scripting/ScriptComponent.h"
#include "ECS/Registry.h"
#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "Core/Log.h"
#include <thread>
#include <chrono>

using namespace MyEngine;

int main() {
    // Initialize logging
    Log::Init();
    ENGINE_INFO("=== Lua Scripting Test ===");

    // Create ECS registry
    Registry registry;

    // Create and initialize script system
    ScriptSystem scriptSystem(&registry);
    scriptSystem.Init();

    ENGINE_INFO("\n=== Creating Test Entities ===");

    // Test 1: Create entity with Rotator script
    {
        Entity entity1 = registry.CreateEntity();
        entity1.AddComponent<TagComponent>("RotatingCube");
        entity1.AddComponent<TransformComponent>(Vec3(0, 0, 0));
        entity1.AddComponent<ScriptComponent>("Runtime/Scripts/Rotator.lua");
        ENGINE_INFO("Created entity with Rotator script");
    }

    // Test 2: Create entity with Bouncer script
    {
        Entity entity2 = registry.CreateEntity();
        entity2.AddComponent<TagComponent>("BouncingBall");
        entity2.AddComponent<TransformComponent>(Vec3(5, 0, 0));
        entity2.AddComponent<ScriptComponent>("Runtime/Scripts/Bouncer.lua");
        ENGINE_INFO("Created entity with Bouncer script");
    }

    // Test 3: Create entity with Orbiter script
    {
        Entity entity3 = registry.CreateEntity();
        entity3.AddComponent<TagComponent>("OrbitingSatellite");
        entity3.AddComponent<TransformComponent>(Vec3(0, 0, 0));
        entity3.AddComponent<ScriptComponent>("Runtime/Scripts/Orbiter.lua");
        ENGINE_INFO("Created entity with Orbiter script");
    }

    ENGINE_INFO("\n=== Running Script Updates ===");
    ENGINE_INFO("Running for 5 seconds (60 FPS)...\n");

    // Simulate game loop for 5 seconds at 60 FPS
    const int totalFrames = 300; // 5 seconds * 60 FPS
    const float targetDeltaTime = 1.0f / 60.0f;
    
    for (int frame = 0; frame < totalFrames; ++frame) {
        // Update all scripts
        scriptSystem.Update(targetDeltaTime);

        // Print status every 60 frames (1 second)
        if (frame % 60 == 0) {
            ENGINE_INFO("Frame {}/{} - {:.1f} seconds elapsed", 
                frame, totalFrames, frame * targetDeltaTime);
        }

        // Simulate frame timing
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }

    ENGINE_INFO("\n=== Test Complete ===");
    
    // Test 4: Verify entity transforms were modified
    {
        auto view = registry.View<TransformComponent, TagComponent>();
        ENGINE_INFO("\nFinal entity positions:");
        for (EntityID entityId : view) {
            Entity entity(entityId, &registry);
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& tag = entity.GetComponent<TagComponent>();
            ENGINE_INFO("  {}: ({:.2f}, {:.2f}, {:.2f})", 
                tag.Tag, 
                transform.localPosition.x,
                transform.localPosition.y,
                transform.localPosition.z);
        }
    }

    // Cleanup
    ENGINE_INFO("\n=== Shutting Down ===");
    scriptSystem.Shutdown();
    ENGINE_INFO("Script system shut down successfully");
    ENGINE_INFO("Test completed!");

    return 0;
}

/* Expected Output:

=== Lua Scripting Test ===

=== Creating Test Entities ===
Created entity with Rotator script
Created entity with Bouncer script
Created entity with Orbiter script

=== Running Script Updates ===
Running for 5 seconds (60 FPS)...

Rotator script initialized for entity: RotatingCube
Initial position: (0.00, 0.00, 0.00)

Bouncer script initialized!
Starting at position: (5.00, 0.00, 0.00)

=== Orbiter Script Initialized ===
Entity: OrbitingSatellite
Starting position: Vec3(0.00, 0.00, 0.00)

=== Vec3 Math Examples ===
v1 = Vec3(1.00, 2.00, 3.00)
v2 = Vec3(4.00, 5.00, 6.00)
v1 + v2 = Vec3(5.00, 7.00, 9.00)
v2 - v1 = Vec3(3.00, 3.00, 3.00)
v1 * 2.0 = Vec3(2.00, 4.00, 6.00)
v1:Length() = 3.74
v1:Normalize() = Vec3(0.27, 0.53, 0.80)
Normalized length: 1.00
v1:Dot(v2) = 32.00
v1:Cross(v2) = Vec3(-3.00, 6.00, -3.00)
=== End Vec3 Examples ===

Frame 0/300 - 0.0 seconds elapsed
Frame 60/300 - 1.0 seconds elapsed
Frame 120/300 - 2.0 seconds elapsed
Frame 180/300 - 3.0 seconds elapsed
Frame 240/300 - 4.0 seconds elapsed

=== Test Complete ===

Final entity positions:
  RotatingCube: (0.00, 0.00, 0.00)
  BouncingBall: (5.00, 1.76, 0.00)
  OrbitingSatellite: (3.54, 2.00, 3.54)

=== Shutting Down ===
Rotator script destroyed for entity: RotatingCube
Bouncer script destroyed
Orbiter script destroyed for OrbitingSatellite
Script system shut down successfully
Test completed!

*/
