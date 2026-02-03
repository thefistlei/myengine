/******************************************************************************
 * File: ScriptSystem.h
 * Author: AI Assistant
 * Created: 2026-02-02
 * Description: System for managing and updating Lua scripts
 ******************************************************************************/

#pragma once

#include "ECS/Registry.h"

struct lua_State;

namespace MyEngine {

/**
 * @brief System that manages script lifecycle and updates
 */
class ScriptSystem {
public:
    ScriptSystem(Registry* registry);
    ~ScriptSystem();

    void Init();
    void Update(float deltaTime);
    void Shutdown();

    /**
     * @brief Get the Lua state
     */
    lua_State* GetLuaState() const { return m_luaState; }

private:
    Registry* m_registry;
    lua_State* m_luaState;
};

} // namespace MyEngine
