/******************************************************************************
 * File: LuaVM.cpp
 * Author: AI Assistant
 * Created: 2026-02-02
 * Description: Lua Virtual Machine implementation
 ******************************************************************************/

#include "LuaVM.h"
#include "Core/Log.h"

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

#ifdef LUA_SCRIPTING_ENABLED

// Static instance
static LuaVM* s_Instance = nullptr;

// Lua allocator with memory tracking
static void* LuaAllocator(void* ud, void* ptr, size_t osize, size_t nsize) {
    LuaVM* vm = static_cast<LuaVM*>(ud);
    
    if (nsize == 0) {
        // Free memory
        if (ptr) {
            vm->m_currentMemoryUsage -= osize;
            free(ptr);
        }
        return nullptr;
    } else {
        // Allocate or reallocate
        size_t memDelta = nsize - osize;
        if (vm->m_currentMemoryUsage + memDelta > vm->m_config.maxMemoryMB * 1024 * 1024) {
            ENGINE_ERROR("Lua VM memory limit exceeded!");
            return nullptr;
        }
        
        void* newPtr = realloc(ptr, nsize);
        if (newPtr) {
            vm->m_currentMemoryUsage += memDelta;
        }
        return newPtr;
    }
}

// Lua panic handler
static int LuaPanicHandler(lua_State* L) {
    const char* msg = lua_tostring(L, -1);
    ENGINE_ERROR("Lua panic: {}", msg ? msg : "unknown error");
    return 0;
}

LuaVM& LuaVM::Instance() {
    if (!s_Instance) {
        s_Instance = new LuaVM();
    }
    return *s_Instance;
}

LuaVM::LuaVM() 
    : m_mainState(nullptr)
    , m_initialized(false)
    , m_currentMemoryUsage(0) {
}

LuaVM::~LuaVM() {
    Shutdown();
}

bool LuaVM::Initialize(const LuaVMConfig& config) {
    if (m_initialized) {
        ENGINE_WARN("LuaVM already initialized");
        return true;
    }

    ENGINE_INFO("Initializing Lua VM...");
    m_config = config;

    // Create main Lua state with custom allocator
    m_mainState = lua_newstate(LuaAllocator, this);
    if (!m_mainState) {
        ENGINE_ERROR("Failed to create Lua state");
        return false;
    }

    // Set panic handler
    lua_atpanic(m_mainState, LuaPanicHandler);

    // Open standard libraries
    luaL_openlibs(m_mainState);

    // Configure garbage collector
    lua_gc(m_mainState, LUA_GCSETPAUSE, m_config.gcPause);
    lua_gc(m_mainState, LUA_GCSETSTEPMUL, m_config.gcStepMultiplier);

    // Add search paths
    for (const auto& path : m_config.searchPaths) {
        AddSearchPath(path);
    }

    m_initialized = true;
    ENGINE_INFO("Lua VM initialized successfully");
    ENGINE_INFO("  Memory limit: {} MB", m_config.maxMemoryMB);
    ENGINE_INFO("  GC pause: {}%", m_config.gcPause);
    ENGINE_INFO("  GC step multiplier: {}", m_config.gcStepMultiplier);

    return true;
}

void LuaVM::Shutdown() {
    if (!m_initialized) {
        return;
    }

    ENGINE_INFO("Shutting down Lua VM...");
    
    if (m_mainState) {
        lua_close(m_mainState);
        m_mainState = nullptr;
    }

    m_initialized = false;
    m_currentMemoryUsage = 0;
    ENGINE_INFO("Lua VM shut down");
}

bool LuaVM::ExecuteFile(const char* filename) {
    if (!m_initialized) {
        ENGINE_ERROR("LuaVM not initialized");
        return false;
    }

    // Load and execute the file
    if (luaL_dofile(m_mainState, filename) != LUA_OK) {
        const char* error = lua_tostring(m_mainState, -1);
        ReportError(std::string("Failed to execute file '") + filename + "': " + error);
        lua_pop(m_mainState, 1); // Pop error message
        return false;
    }

    return true;
}

bool LuaVM::ExecuteString(const char* code, const char* chunkName) {
    if (!m_initialized) {
        ENGINE_ERROR("LuaVM not initialized");
        return false;
    }

    if (luaL_loadbuffer(m_mainState, code, strlen(code), chunkName) != LUA_OK) {
        const char* error = lua_tostring(m_mainState, -1);
        ReportError(std::string("Failed to load chunk: ") + error);
        lua_pop(m_mainState, 1);
        return false;
    }

    if (lua_pcall(m_mainState, 0, LUA_MULTRET, 0) != LUA_OK) {
        const char* error = lua_tostring(m_mainState, -1);
        ReportError(std::string("Failed to execute chunk: ") + error);
        lua_pop(m_mainState, 1);
        return false;
    }

    return true;
}

void LuaVM::SetErrorHandler(ErrorHandler handler) {
    m_errorHandler = handler;
}

void LuaVM::ReportError(const std::string& error) {
    ENGINE_ERROR("Lua Error: {}", error);
    
    if (m_errorHandler) {
        m_errorHandler(error);
    }
}

void LuaVM::AddSearchPath(const std::string& path) {
    if (!m_initialized) {
        ENGINE_WARN("Cannot add search path - LuaVM not initialized");
        return;
    }

    // Get package.path
    lua_getglobal(m_mainState, "package");
    lua_getfield(m_mainState, -1, "path");
    std::string currentPath = lua_tostring(m_mainState, -1);
    lua_pop(m_mainState, 1);

    // Append new path
    std::string newPath = currentPath + ";" + path + "/?.lua;" + path + "/?/init.lua";
    lua_pushstring(m_mainState, newPath.c_str());
    lua_setfield(m_mainState, -2, "path");
    lua_pop(m_mainState, 1); // Pop package table

    ENGINE_INFO("Added Lua search path: {}", path);
}

size_t LuaVM::GetMemoryUsage() const {
    return m_currentMemoryUsage;
}

void LuaVM::CollectGarbage() {
    if (m_initialized) {
        lua_gc(m_mainState, LUA_GCCOLLECT, 0);
    }
}

#else
// Stub implementations when Lua is not available

static LuaVM* s_Instance = nullptr;

LuaVM& LuaVM::Instance() {
    if (!s_Instance) {
        s_Instance = new LuaVM();
    }
    return *s_Instance;
}

LuaVM::LuaVM() : m_mainState(nullptr), m_initialized(false), m_currentMemoryUsage(0) {}
LuaVM::~LuaVM() {}

bool LuaVM::Initialize(const LuaVMConfig& config) {
    ENGINE_WARN("Lua scripting not enabled - VM initialization skipped");
    return false;
}

void LuaVM::Shutdown() {}
bool LuaVM::ExecuteFile(const char* filename) { return false; }
bool LuaVM::ExecuteString(const char* code, const char* chunkName) { return false; }
void LuaVM::SetErrorHandler(ErrorHandler handler) {}
void LuaVM::ReportError(const std::string& error) {}
size_t LuaVM::GetMemoryUsage() const { return 0; }
void LuaVM::CollectGarbage() {}
void LuaVM::AddSearchPath(const std::string& path) {}

#endif // LUA_SCRIPTING_ENABLED

} // namespace MyEngine
