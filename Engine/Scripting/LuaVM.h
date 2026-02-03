/******************************************************************************
 * File: LuaVM.h
 * Author: AI Assistant
 * Created: 2026-02-02
 * Description: Lua Virtual Machine manager
 ******************************************************************************/

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>

struct lua_State;

namespace MyEngine {

/**
 * @brief Lua VM configuration
 */
struct LuaVMConfig {
    size_t initialMemoryMB = 64;
    size_t maxMemoryMB = 512;
    bool enableDebugHook = false;
    int gcStepMultiplier = 200;
    int gcPause = 200;
    std::vector<std::string> searchPaths;
};

/**
 * @brief Lua Virtual Machine Manager (Singleton)
 * 
 * Manages the Lua VM with custom memory allocation and tracking.
 * Provides script execution and error handling.
 */
class LuaVM {
public:
    static LuaVM& Instance();
    
    bool Initialize(const LuaVMConfig& config);
    void Shutdown();
    bool IsInitialized() const { return m_initialized; }
    
    lua_State* GetMainState() const { return m_mainState; }
    
    // Script execution
    bool ExecuteFile(const char* filename);
    bool ExecuteString(const char* code, const char* chunkName = "");
    
    // Error handling
    using ErrorHandler = std::function<void(const std::string&)>;
    void SetErrorHandler(ErrorHandler handler);
    void ReportError(const std::string& error);
    
    // Memory management
    size_t GetMemoryUsage() const;
    void CollectGarbage();
    
    // Search paths
    void AddSearchPath(const std::string& path);
    
private:
    LuaVM();
    ~LuaVM();
    LuaVM(const LuaVM&) = delete;
    LuaVM& operator=(const LuaVM&) = delete;
    
    lua_State* m_mainState;
    bool m_initialized;
    LuaVMConfig m_config;
    ErrorHandler m_errorHandler;
    size_t m_currentMemoryUsage;
    
    friend void* LuaAllocator(void* ud, void* ptr, size_t osize, size_t nsize);
};

} // namespace MyEngine
