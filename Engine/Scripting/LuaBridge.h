/******************************************************************************
 * File: LuaBridge.h
 * Author: AI Assistant
 * Created: 2026-02-02
 * Description: Bridge between C++ engine classes and Lua
 ******************************************************************************/

#pragma once

#include "Math/MathTypes.h"
#include "ECS/Entity.h"

struct lua_State;

namespace MyEngine {

/**
 * @brief Bridge class for binding C++ types to Lua
 */
class LuaBridge {
public:
    /**
     * @brief Register all engine bindings with Lua
     */
    static void RegisterBindings(lua_State* L);

    // Entity bindings
    static void PushEntity(lua_State* L, Entity entity);
    static Entity CheckEntity(lua_State* L, int index);

    // Vec3 bindings
    static void PushVec3(lua_State* L, const Vec3& v);
    static Vec3 CheckVec3(lua_State* L, int index);
    static void RegisterVec3(lua_State* L);

    // Transform bindings
    static void RegisterTransform(lua_State* L);
    
    // Entity bindings
    static void RegisterEntity(lua_State* L);

private:
    // Vec3 Lua methods
    static int Vec3_new(lua_State* L);
    static int Vec3_add(lua_State* L);
    static int Vec3_sub(lua_State* L);
    static int Vec3_mul(lua_State* L);
    static int Vec3_length(lua_State* L);
    static int Vec3_normalize(lua_State* L);
    static int Vec3_dot(lua_State* L);
    static int Vec3_cross(lua_State* L);
    static int Vec3_tostring(lua_State* L);
    static int Vec3_index(lua_State* L);
    static int Vec3_newindex(lua_State* L);

    // Entity Lua methods
    static int Entity_GetPosition(lua_State* L);
    static int Entity_SetPosition(lua_State* L);
    static int Entity_GetRotation(lua_State* L);
    static int Entity_SetRotation(lua_State* L);
    static int Entity_GetScale(lua_State* L);
    static int Entity_SetScale(lua_State* L);
    static int Entity_GetTag(lua_State* L);
    static int Entity_SetTag(lua_State* L);
};

} // namespace MyEngine
