/******************************************************************************
 * File: ScriptComponent.h
 * Author: AI Assistant
 * Created: 2026-02-02
 * Description: Script component for attaching Lua scripts to entities
 ******************************************************************************/

#pragma once

#include <string>
#include <memory>
#include <unordered_map>

struct lua_State;

namespace MyEngine {

class Entity;

/**
 * @brief Script component that attaches Lua scripts to entities
 * 
 * Each script has its own environment table to isolate variables.
 * Scripts can access the entity through the global 'self' variable.
 */
struct ScriptComponent {
    std::string scriptPath;           // Path to the Lua script file
    bool enabled = true;              // Whether the script is active
    bool initialized = false;         // Whether OnInit has been called
    
    // Script environment (stored as registry reference)
    int envRef = -1;                  // Reference to the script's environment table
    
    // Lifecycle function references
    int onInitRef = -1;               // Reference to OnInit function
    int onUpdateRef = -1;             // Reference to OnUpdate function
    int onDestroyRef = -1;            // Reference to OnDestroy function
    
    ScriptComponent() = default;
    ScriptComponent(const std::string& path) : scriptPath(path) {}
    
    /**
     * @brief Load the script from file
     */
    bool Load(lua_State* L, Entity entity);
    
    /**
     * @brief Call the OnInit function
     */
    void CallOnInit(lua_State* L, Entity entity);
    
    /**
     * @brief Call the OnUpdate function
     */
    void CallOnUpdate(lua_State* L, Entity entity, float deltaTime);
    
    /**
     * @brief Call the OnDestroy function
     */
    void CallOnDestroy(lua_State* L, Entity entity);
    
    /**
     * @brief Clean up Lua references
     */
    void Cleanup(lua_State* L);
};

} // namespace MyEngine
