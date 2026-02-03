/******************************************************************************
 * File: Module.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Module system with registration and dependency management
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace MyEngine {

/**
 * @brief Module interface
 */
class IModule {
public:
    virtual ~IModule() = default;

    /**
     * @brief Get module name
     */
    virtual const char* GetName() const = 0;

    /**
     * @brief Get module version
     */
    virtual const char* GetVersion() const = 0;

    /**
     * @brief Get module dependencies
     */
    virtual std::vector<std::string> GetDependencies() const { return {}; }

    /**
     * @brief Initialize module
     */
    virtual bool Initialize() = 0;

    /**
     * @brief Shutdown module
     */
    virtual void Shutdown() = 0;

    /**
     * @brief Update module (called per frame)
     */
    virtual void Update(float deltaTime) {}
};

/**
 * @brief Module registry - manages all engine modules
 */
class ModuleRegistry {
public:
    static ModuleRegistry& Get() {
        static ModuleRegistry instance;
        return instance;
    }

    /**
     * @brief Register a module
     */
    void RegisterModule(const std::string& name, std::shared_ptr<IModule> module);

    /**
     * @brief Initialize all modules in dependency order
     */
    bool InitializeAll();

    /**
     * @brief Shutdown all modules in reverse order
     */
    void ShutdownAll();

    /**
     * @brief Update all modules
     */
    void UpdateAll(float deltaTime);

    /**
     * @brief Get module by name
     */
    template<typename T>
    std::shared_ptr<T> GetModule(const std::string& name) {
        auto it = m_Modules.find(name);
        if (it != m_Modules.end()) {
            return std::dynamic_pointer_cast<T>(it->second);
        }
        return nullptr;
    }

private:
    ModuleRegistry() = default;

    /**
     * @brief Resolve module dependencies and return initialization order
     */
    std::vector<std::string> ResolveDependencies();

    /**
     * @brief Check for circular dependencies
     */
    bool HasCircularDependency(const std::string& module, 
                                std::vector<std::string>& visited);

private:
    std::unordered_map<std::string, std::shared_ptr<IModule>> m_Modules;
    std::vector<std::string> m_InitializationOrder;
};

/**
 * @brief Module registration helper macro
 */
#define REGISTER_MODULE(ModuleClass) \
    namespace { \
        struct ModuleClass##Registrar { \
            ModuleClass##Registrar() { \
                MyEngine::ModuleRegistry::Get().RegisterModule( \
                    #ModuleClass, std::make_shared<ModuleClass>()); \
            } \
        }; \
        static ModuleClass##Registrar g_##ModuleClass##Registrar; \
    }

} // namespace MyEngine
