/******************************************************************************
 * File: ScriptSystem.cpp
 * Author: AI Assistant
 * Created: 2026-02-02
 * Description: Script system implementation
 ******************************************************************************/

#include "ScriptSystem.h"
#include "ScriptComponent.h"
#include "LuaVM.h"
#include "LuaBridge.h"
#include "ECS/Entity.h"
#include "Core/Log.h"

namespace MyEngine {

ScriptSystem::ScriptSystem(Registry* registry)
    : m_registry(registry)
    , m_luaState(nullptr) {
}

ScriptSystem::~ScriptSystem() {
    Shutdown();
}

void ScriptSystem::Init() {
#ifdef LUA_SCRIPTING_ENABLED
    ENGINE_INFO("Initializing Script System...");

    // Initialize Lua VM
    LuaVMConfig config;
    config.initialMemoryMB = 64;
    config.maxMemoryMB = 512;
    config.gcStepMultiplier = 200;
    config.gcPause = 200;
    config.searchPaths = { "Runtime/Scripts", "Assets/Scripts" };

    if (!LuaVM::Instance().Initialize(config)) {
        ENGINE_ERROR("Failed to initialize Lua VM");
        return;
    }

    m_luaState = LuaVM::Instance().GetMainState();

    // Register engine bindings
    LuaBridge::RegisterBindings(m_luaState);

    ENGINE_INFO("Script System initialized");
#else
    ENGINE_WARN("Lua scripting not enabled - Script System not initialized");
#endif
}

void ScriptSystem::Update(float deltaTime) {
#ifdef LUA_SCRIPTING_ENABLED
    if (!m_luaState) {
        return;
    }

    // Get all entities with ScriptComponent
    auto entities = m_registry->GetEntitiesWith<ScriptComponent>();

    for (EntityID entityId : entities) {
        Entity entity(entityId, m_registry);
        auto& script = entity.GetComponent<ScriptComponent>();

        if (!script.enabled) {
            continue;
        }

        // Load script if not loaded
        if (script.envRef == -1) {
            if (!script.Load(m_luaState, entity)) {
                ENGINE_ERROR("Failed to load script for entity {}", entityId);
                script.enabled = false;
                continue;
            }
        }

        // Call OnInit if not initialized
        if (!script.initialized) {
            script.CallOnInit(m_luaState, entity);
        }

        // Call OnUpdate
        script.CallOnUpdate(m_luaState, entity, deltaTime);
    }
#endif
}

void ScriptSystem::Shutdown() {
#ifdef LUA_SCRIPTING_ENABLED
    ENGINE_INFO("Shutting down Script System...");

    if (!m_luaState) {
        return;
    }

    // Clean up all script components
    auto entities = m_registry->GetEntitiesWith<ScriptComponent>();
    for (EntityID entityId : entities) {
        Entity entity(entityId, m_registry);
        auto& script = entity.GetComponent<ScriptComponent>();
        script.CallOnDestroy(m_luaState, entity);
        script.Cleanup(m_luaState);
    }

    // Shutdown Lua VM
    LuaVM::Instance().Shutdown();
    m_luaState = nullptr;

    ENGINE_INFO("Script System shut down");
#endif
}

} // namespace MyEngine
