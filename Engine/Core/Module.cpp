/******************************************************************************
 * File: Module.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Module system implementation
 ******************************************************************************/

#include "Module.h"
#include "Log.h"
#include <algorithm>
#include <set>
#include <functional>

namespace MyEngine {

void ModuleRegistry::RegisterModule(const std::string& name, std::shared_ptr<IModule> module) {
    if (m_Modules.find(name) != m_Modules.end()) {
        ENGINE_WARN("Module '{}' already registered", name.c_str());
        return;
    }

    m_Modules[name] = module;
    ENGINE_INFO("Registered module: {} v{}", name.c_str(), module->GetVersion());
}

bool ModuleRegistry::InitializeAll() {
    ENGINE_INFO("Initializing all modules...");

    // Resolve dependencies
    m_InitializationOrder = ResolveDependencies();
    if (m_InitializationOrder.empty() && !m_Modules.empty()) {
        ENGINE_ERROR("Failed to resolve module dependencies (circular dependency?)");
        return false;
    }

    // Initialize in dependency order
    for (const auto& moduleName : m_InitializationOrder) {
        auto it = m_Modules.find(moduleName);
        if (it != m_Modules.end()) {
            ENGINE_INFO("Initializing module: {}", moduleName.c_str());
            if (!it->second->Initialize()) {
                ENGINE_ERROR("Failed to initialize module: {}", moduleName.c_str());
                return false;
            }
        }
    }

    ENGINE_INFO("All modules initialized successfully");
    return true;
}

void ModuleRegistry::ShutdownAll() {
    ENGINE_INFO("Shutting down all modules...");

    // Shutdown in reverse order
    for (auto it = m_InitializationOrder.rbegin(); it != m_InitializationOrder.rend(); ++it) {
        auto moduleIt = m_Modules.find(*it);
        if (moduleIt != m_Modules.end()) {
            ENGINE_INFO("Shutting down module: {}", it->c_str());
            moduleIt->second->Shutdown();
        }
    }

    ENGINE_INFO("All modules shut down");
}

void ModuleRegistry::UpdateAll(float deltaTime) {
    for (const auto& moduleName : m_InitializationOrder) {
        auto it = m_Modules.find(moduleName);
        if (it != m_Modules.end()) {
            it->second->Update(deltaTime);
        }
    }
}

std::vector<std::string> ModuleRegistry::ResolveDependencies() {
    std::vector<std::string> order;
    std::set<std::string> visited;
    std::set<std::string> resolved;

    // Topological sort
    std::function<bool(const std::string&)> visit = [&](const std::string& moduleName) -> bool {
        if (resolved.find(moduleName) != resolved.end()) {
            return true;
        }

        if (visited.find(moduleName) != visited.end()) {
            // Circular dependency detected
            ENGINE_ERROR("Circular dependency detected at module: {}", moduleName.c_str());
            return false;
        }

        visited.insert(moduleName);

        auto it = m_Modules.find(moduleName);
        if (it != m_Modules.end()) {
            auto deps = it->second->GetDependencies();
            for (const auto& dep : deps) {
                if (!visit(dep)) {
                    return false;
                }
            }
        }

        visited.erase(moduleName);
        resolved.insert(moduleName);
        order.push_back(moduleName);
        return true;
    };

    // Visit all modules
    for (const auto& pair : m_Modules) {
        if (!visit(pair.first)) {
            return {};  // Return empty on error
        }
    }

    return order;
}

bool ModuleRegistry::HasCircularDependency(const std::string& module, 
                                           std::vector<std::string>& visited) {
    if (std::find(visited.begin(), visited.end(), module) != visited.end()) {
        return true;
    }

    visited.push_back(module);

    auto it = m_Modules.find(module);
    if (it != m_Modules.end()) {
        auto deps = it->second->GetDependencies();
        for (const auto& dep : deps) {
            if (HasCircularDependency(dep, visited)) {
                return true;
            }
        }
    }

    visited.pop_back();
    return false;
}

} // namespace MyEngine
