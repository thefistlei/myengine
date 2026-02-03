/******************************************************************************
 * File: ScriptComponent.cpp
 * Author: AI Assistant
 * Created: 2026-02-02
 * Description: Script component implementation
 ******************************************************************************/

#include "ScriptComponent.h"
#include "LuaBridge.h"
#include "Core/Log.h"
#include "ECS/Entity.h"

#ifdef LUA_SCRIPTING_ENABLED
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

// Define LUA_OK if not defined (for Lua 5.1/5.2 compatibility)
#ifndef LUA_OK
#define LUA_OK 0
#endif
#endif // LUA_SCRIPTING_ENABLED

namespace MyEngine {

bool ScriptComponent::Load(lua_State* L, Entity entity) {
#ifdef LUA_SCRIPTING_ENABLED
    if (scriptPath.empty()) {
        ENGINE_ERROR("ScriptComponent: Empty script path");
        return false;
    }

    // Load the script file
    if (luaL_loadfile(L, scriptPath.c_str()) != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        ENGINE_ERROR("Failed to load script '{}': {}", scriptPath, error);
        lua_pop(L, 1);
        return false;
    }

    // Create a new environment table for this script instance
    lua_newtable(L);
    
    // Set the environment's metatable to access globals
    lua_newtable(L);
    lua_getglobal(L, "_G");
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);
    
    // Store reference to environment
    lua_pushvalue(L, -1);
    envRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    // Set 'self' to the entity in the environment
    LuaBridge::PushEntity(L, entity);
    lua_setfield(L, -2, "self");
    
    // Set the loaded chunk's environment and execute it
    lua_setupvalue(L, -2, 1);
    
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        ENGINE_ERROR("Failed to execute script '{}': {}", scriptPath, error);
        lua_pop(L, 1);
        return false;
    }

    // Get the environment table
    lua_rawgeti(L, LUA_REGISTRYINDEX, envRef);
    
    // Cache function references
    lua_getfield(L, -1, "OnInit");
    if (lua_isfunction(L, -1)) {
        onInitRef = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        lua_pop(L, 1);
    }
    
    lua_getfield(L, -1, "OnUpdate");
    if (lua_isfunction(L, -1)) {
        onUpdateRef = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        lua_pop(L, 1);
    }
    
    lua_getfield(L, -1, "OnDestroy");
    if (lua_isfunction(L, -1)) {
        onDestroyRef = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        lua_pop(L, 1);
    }
    
    lua_pop(L, 1); // Pop environment table

    ENGINE_INFO("Loaded script: {}", scriptPath);
    return true;
#else
    ENGINE_WARN("Lua scripting not enabled - cannot load script: {}", scriptPath);
    return false;
#endif
}

void ScriptComponent::CallOnInit(lua_State* L, Entity entity) {
#ifdef LUA_SCRIPTING_ENABLED
    if (!enabled || initialized || onInitRef == -1) {
        return;
    }

    // Get function
    lua_rawgeti(L, LUA_REGISTRYINDEX, onInitRef);
    
    // Get environment and set 'self'
    lua_rawgeti(L, LUA_REGISTRYINDEX, envRef);
    LuaBridge::PushEntity(L, entity);
    lua_setfield(L, -2, "self");
    lua_pop(L, 1);
    
    // Call function
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        ENGINE_ERROR("Error in OnInit for '{}': {}", scriptPath, error);
        lua_pop(L, 1);
    }
    
    initialized = true;
#endif
}

void ScriptComponent::CallOnUpdate(lua_State* L, Entity entity, float deltaTime) {
#ifdef LUA_SCRIPTING_ENABLED
    if (!enabled || !initialized || onUpdateRef == -1) {
        return;
    }

    // Get function
    lua_rawgeti(L, LUA_REGISTRYINDEX, onUpdateRef);
    
    // Get environment and set 'self'
    lua_rawgeti(L, LUA_REGISTRYINDEX, envRef);
    LuaBridge::PushEntity(L, entity);
    lua_setfield(L, -2, "self");
    lua_pop(L, 1);
    
    // Push deltaTime argument
    lua_pushnumber(L, deltaTime);
    
    // Call function
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        ENGINE_ERROR("Error in OnUpdate for '{}': {}", scriptPath, error);
        lua_pop(L, 1);
    }
#endif
}

void ScriptComponent::CallOnDestroy(lua_State* L, Entity entity) {
#ifdef LUA_SCRIPTING_ENABLED
    if (!initialized || onDestroyRef == -1) {
        return;
    }

    // Get function
    lua_rawgeti(L, LUA_REGISTRYINDEX, onDestroyRef);
    
    // Get environment and set 'self'
    lua_rawgeti(L, LUA_REGISTRYINDEX, envRef);
    LuaBridge::PushEntity(L, entity);
    lua_setfield(L, -2, "self");
    lua_pop(L, 1);
    
    // Call function
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        ENGINE_ERROR("Error in OnDestroy for '{}': {}", scriptPath, error);
        lua_pop(L, 1);
    }
#endif
}

void ScriptComponent::Cleanup(lua_State* L) {
#ifdef LUA_SCRIPTING_ENABLED
    if (envRef != -1) {
        luaL_unref(L, LUA_REGISTRYINDEX, envRef);
        envRef = -1;
    }
    if (onInitRef != -1) {
        luaL_unref(L, LUA_REGISTRYINDEX, onInitRef);
        onInitRef = -1;
    }
    if (onUpdateRef != -1) {
        luaL_unref(L, LUA_REGISTRYINDEX, onUpdateRef);
        onUpdateRef = -1;
    }
    if (onDestroyRef != -1) {
        luaL_unref(L, LUA_REGISTRYINDEX, onDestroyRef);
        onDestroyRef = -1;
    }
    initialized = false;
#endif
}

} // namespace MyEngine
