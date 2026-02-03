/******************************************************************************
 * File: LuaBridge.cpp
 * Author: AI Assistant
 * Created: 2026-02-02
 * Description: Lua bridge implementation
 ******************************************************************************/

#include "LuaBridge.h"
#include "ECS/Components.h"
#include "Core/Log.h"
#include <cstring>

#ifdef LUA_SCRIPTING_ENABLED
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#endif // LUA_SCRIPTING_ENABLED

namespace MyEngine {

#ifdef LUA_SCRIPTING_ENABLED

// Metatable names
static const char* VEC3_METATABLE = "MyEngine.Vec3";
static const char* ENTITY_METATABLE = "MyEngine.Entity";

// =============================================================================
// Registration
// =============================================================================

void LuaBridge::RegisterBindings(lua_State* L) {
    ENGINE_INFO("Registering Lua bindings...");
    RegisterVec3(L);
    RegisterEntity(L);
    ENGINE_INFO("Lua bindings registered successfully");
}

// =============================================================================
// Vec3 Implementation
// =============================================================================

void LuaBridge::PushVec3(lua_State* L, const Vec3& v) {
    Vec3* userdata = static_cast<Vec3*>(lua_newuserdata(L, sizeof(Vec3)));
    new (userdata) Vec3(v);
    luaL_getmetatable(L, VEC3_METATABLE);
    lua_setmetatable(L, -2);
}

Vec3 LuaBridge::CheckVec3(lua_State* L, int index) {
    Vec3* v = static_cast<Vec3*>(luaL_checkudata(L, index, VEC3_METATABLE));
    return *v;
}

void LuaBridge::RegisterVec3(lua_State* L) {
    // Create metatable
    luaL_newmetatable(L, VEC3_METATABLE);
    
    // __index metamethod
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, Vec3_index);
    lua_settable(L, -3);
    
    // __newindex metamethod
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, Vec3_newindex);
    lua_settable(L, -3);
    
    // __add metamethod
    lua_pushstring(L, "__add");
    lua_pushcfunction(L, Vec3_add);
    lua_settable(L, -3);
    
    // __sub metamethod
    lua_pushstring(L, "__sub");
    lua_pushcfunction(L, Vec3_sub);
    lua_settable(L, -3);
    
    // __mul metamethod
    lua_pushstring(L, "__mul");
    lua_pushcfunction(L, Vec3_mul);
    lua_settable(L, -3);
    
    // __tostring metamethod
    lua_pushstring(L, "__tostring");
    lua_pushcfunction(L, Vec3_tostring);
    lua_settable(L, -3);
    
    // Methods table
    lua_newtable(L);
    
    lua_pushcfunction(L, Vec3_length);
    lua_setfield(L, -2, "Length");
    
    lua_pushcfunction(L, Vec3_normalize);
    lua_setfield(L, -2, "Normalize");
    
    lua_pushcfunction(L, Vec3_dot);
    lua_setfield(L, -2, "Dot");
    
    lua_pushcfunction(L, Vec3_cross);
    lua_setfield(L, -2, "Cross");
    
    lua_setfield(L, -2, "methods");
    
    lua_pop(L, 1); // Pop metatable
    
    // Create global Vec3 constructor
    lua_pushcfunction(L, Vec3_new);
    lua_setglobal(L, "Vec3");
}

int LuaBridge::Vec3_new(lua_State* L) {
    float x = luaL_optnumber(L, 1, 0.0f);
    float y = luaL_optnumber(L, 2, 0.0f);
    float z = luaL_optnumber(L, 3, 0.0f);
    PushVec3(L, Vec3(x, y, z));
    return 1;
}

int LuaBridge::Vec3_add(lua_State* L) {
    Vec3 a = CheckVec3(L, 1);
    Vec3 b = CheckVec3(L, 2);
    PushVec3(L, a + b);
    return 1;
}

int LuaBridge::Vec3_sub(lua_State* L) {
    Vec3 a = CheckVec3(L, 1);
    Vec3 b = CheckVec3(L, 2);
    PushVec3(L, a - b);
    return 1;
}

int LuaBridge::Vec3_mul(lua_State* L) {
    Vec3 v = CheckVec3(L, 1);
    float scalar = luaL_checknumber(L, 2);
    PushVec3(L, v * scalar);
    return 1;
}

int LuaBridge::Vec3_length(lua_State* L) {
    Vec3 v = CheckVec3(L, 1);
    lua_pushnumber(L, v.Length());
    return 1;
}

int LuaBridge::Vec3_normalize(lua_State* L) {
    Vec3 v = CheckVec3(L, 1);
    PushVec3(L, v.Normalized());
    return 1;
}

int LuaBridge::Vec3_dot(lua_State* L) {
    Vec3 a = CheckVec3(L, 1);
    Vec3 b = CheckVec3(L, 2);
    lua_pushnumber(L, Vec3::Dot(a, b));
    return 1;
}

int LuaBridge::Vec3_cross(lua_State* L) {
    Vec3 a = CheckVec3(L, 1);
    Vec3 b = CheckVec3(L, 2);
    PushVec3(L, Vec3::Cross(a, b));
    return 1;
}

int LuaBridge::Vec3_tostring(lua_State* L) {
    Vec3 v = CheckVec3(L, 1);
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Vec3(%.2f, %.2f, %.2f)", v.x, v.y, v.z);
    lua_pushstring(L, buffer);
    return 1;
}

int LuaBridge::Vec3_index(lua_State* L) {
    Vec3* v = static_cast<Vec3*>(luaL_checkudata(L, 1, VEC3_METATABLE));
    const char* key = luaL_checkstring(L, 2);
    
    if (strcmp(key, "x") == 0) {
        lua_pushnumber(L, v->x);
    } else if (strcmp(key, "y") == 0) {
        lua_pushnumber(L, v->y);
    } else if (strcmp(key, "z") == 0) {
        lua_pushnumber(L, v->z);
    } else {
        // Check methods table
        luaL_getmetatable(L, VEC3_METATABLE);
        lua_getfield(L, -1, "methods");
        lua_getfield(L, -1, key);
        if (lua_isnil(L, -1)) {
            luaL_error(L, "Vec3 has no property or method '%s'", key);
        }
    }
    return 1;
}

int LuaBridge::Vec3_newindex(lua_State* L) {
    Vec3* v = static_cast<Vec3*>(luaL_checkudata(L, 1, VEC3_METATABLE));
    const char* key = luaL_checkstring(L, 2);
    float value = luaL_checknumber(L, 3);
    
    if (strcmp(key, "x") == 0) {
        v->x = value;
    } else if (strcmp(key, "y") == 0) {
        v->y = value;
    } else if (strcmp(key, "z") == 0) {
        v->z = value;
    } else {
        luaL_error(L, "Vec3 has no writable property '%s'", key);
    }
    return 0;
}

// =============================================================================
// Entity Implementation
// =============================================================================

void LuaBridge::PushEntity(lua_State* L, Entity entity) {
    Entity* userdata = static_cast<Entity*>(lua_newuserdata(L, sizeof(Entity)));
    new (userdata) Entity(entity);
    luaL_getmetatable(L, ENTITY_METATABLE);
    lua_setmetatable(L, -2);
}

Entity LuaBridge::CheckEntity(lua_State* L, int index) {
    Entity* entity = static_cast<Entity*>(luaL_checkudata(L, index, ENTITY_METATABLE));
    return *entity;
}

void LuaBridge::RegisterEntity(lua_State* L) {
    // Create metatable
    luaL_newmetatable(L, ENTITY_METATABLE);
    
    // Methods table
    lua_newtable(L);
    
    lua_pushcfunction(L, Entity_GetPosition);
    lua_setfield(L, -2, "GetPosition");
    
    lua_pushcfunction(L, Entity_SetPosition);
    lua_setfield(L, -2, "SetPosition");
    
    lua_pushcfunction(L, Entity_GetRotation);
    lua_setfield(L, -2, "GetRotation");
    
    lua_pushcfunction(L, Entity_SetRotation);
    lua_setfield(L, -2, "SetRotation");
    
    lua_pushcfunction(L, Entity_GetScale);
    lua_setfield(L, -2, "GetScale");
    
    lua_pushcfunction(L, Entity_SetScale);
    lua_setfield(L, -2, "SetScale");
    
    lua_pushcfunction(L, Entity_GetTag);
    lua_setfield(L, -2, "GetTag");
    
    lua_pushcfunction(L, Entity_SetTag);
    lua_setfield(L, -2, "SetTag");
    
    lua_setfield(L, -2, "__index");
    
    lua_pop(L, 1); // Pop metatable
}

int LuaBridge::Entity_GetPosition(lua_State* L) {
    Entity entity = CheckEntity(L, 1);
    if (!entity.HasComponent<TransformComponent>()) {
        return luaL_error(L, "Entity does not have TransformComponent");
    }
    auto& transform = entity.GetComponent<TransformComponent>();
    PushVec3(L, transform.localPosition);
    return 1;
}

int LuaBridge::Entity_SetPosition(lua_State* L) {
    Entity entity = CheckEntity(L, 1);
    Vec3 position = CheckVec3(L, 2);
    
    if (!entity.HasComponent<TransformComponent>()) {
        return luaL_error(L, "Entity does not have TransformComponent");
    }
    
    auto& transform = entity.GetComponent<TransformComponent>();
    transform.localPosition = position;
    transform.UpdateLocalMatrix();
    return 0;
}

int LuaBridge::Entity_GetRotation(lua_State* L) {
    Entity entity = CheckEntity(L, 1);
    if (!entity.HasComponent<TransformComponent>()) {
        return luaL_error(L, "Entity does not have TransformComponent");
    }
    auto& transform = entity.GetComponent<TransformComponent>();
    // For simplicity, return euler angles (not implemented yet)
    PushVec3(L, Vec3(0, 0, 0));
    return 1;
}

int LuaBridge::Entity_SetRotation(lua_State* L) {
    Entity entity = CheckEntity(L, 1);
    Vec3 rotation = CheckVec3(L, 2);
    
    if (!entity.HasComponent<TransformComponent>()) {
        return luaL_error(L, "Entity does not have TransformComponent");
    }
    
    // TODO: Implement rotation setting
    ENGINE_WARN("Entity:SetRotation not fully implemented yet");
    return 0;
}

int LuaBridge::Entity_GetScale(lua_State* L) {
    Entity entity = CheckEntity(L, 1);
    if (!entity.HasComponent<TransformComponent>()) {
        return luaL_error(L, "Entity does not have TransformComponent");
    }
    auto& transform = entity.GetComponent<TransformComponent>();
    PushVec3(L, transform.localScale);
    return 1;
}

int LuaBridge::Entity_SetScale(lua_State* L) {
    Entity entity = CheckEntity(L, 1);
    Vec3 scale = CheckVec3(L, 2);
    
    if (!entity.HasComponent<TransformComponent>()) {
        return luaL_error(L, "Entity does not have TransformComponent");
    }
    
    auto& transform = entity.GetComponent<TransformComponent>();
    transform.localScale = scale;
    transform.UpdateLocalMatrix();
    return 0;
}

int LuaBridge::Entity_GetTag(lua_State* L) {
    Entity entity = CheckEntity(L, 1);
    if (!entity.HasComponent<TagComponent>()) {
        lua_pushstring(L, "Unnamed");
        return 1;
    }
    auto& tag = entity.GetComponent<TagComponent>();
    lua_pushstring(L, tag.Tag.c_str());
    return 1;
}

int LuaBridge::Entity_SetTag(lua_State* L) {
    Entity entity = CheckEntity(L, 1);
    const char* tag = luaL_checkstring(L, 2);
    
    if (!entity.HasComponent<TagComponent>()) {
        entity.AddComponent<TagComponent>(tag);
    } else {
        auto& tagComp = entity.GetComponent<TagComponent>();
        tagComp.Tag = tag;
    }
    return 0;
}

#else
// Stub implementations when Lua is not available

void LuaBridge::RegisterBindings(lua_State* L) {}
void LuaBridge::PushEntity(lua_State* L, Entity entity) {}
Entity LuaBridge::CheckEntity(lua_State* L, int index) { return Entity(); }
void LuaBridge::PushVec3(lua_State* L, const Vec3& v) {}
Vec3 LuaBridge::CheckVec3(lua_State* L, int index) { return Vec3(); }
void LuaBridge::RegisterVec3(lua_State* L) {}
void LuaBridge::RegisterTransform(lua_State* L) {}
void LuaBridge::RegisterEntity(lua_State* L) {}

#endif // LUA_SCRIPTING_ENABLED

} // namespace MyEngine
