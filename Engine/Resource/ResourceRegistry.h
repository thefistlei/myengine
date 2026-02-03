/******************************************************************************
 * File: ResourceRegistry.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Resource registry for managing resources with handles
 ******************************************************************************/

#pragma once

#include <unordered_map>
#include <memory>
#include "ResourceHandle.h"
#include "Core/Log.h"

namespace MyEngine {

/**
 * @brief Entry in the resource registry
 */
template<typename T>
struct ResourceEntry {
    std::shared_ptr<T> Resource;
    uint32_t Generation;
};

/**
 * @brief Registry for managing resources with type-safe handles
 */
template<typename T>
class ResourceRegistry {
public:
    /**
     * @brief Register a new resource
     */
    Handle<T> Register(std::shared_ptr<T> resource) {
        UUID id;
        m_Resources[id] = { resource, 0 };
        return Handle<T>(id, 0);
    }
    
    /**
     * @brief Get a resource by handle
     */
    std::shared_ptr<T> Get(Handle<T> handle) {
        auto it = m_Resources.find(handle.GetID());
        if (it != m_Resources.end()) {
            if (it->second.Generation == handle.GetGeneration()) {
                return it->second.Resource;
            }
        }
        return nullptr;
    }
    
    /**
     * @brief Unregister a resource (invalidates handle)
     */
    void Unregister(Handle<T> handle) {
        auto it = m_Resources.find(handle.GetID());
        if (it != m_Resources.end()) {
            if (it->second.Generation == handle.GetGeneration()) {
                it->second.Generation++;
                it->second.Resource.reset();
                m_Resources.erase(it);
            }
        }
    }
    
    /**
     * @brief Check if a handle is valid
     */
    bool IsValid(Handle<T> handle) const {
        auto it = m_Resources.find(handle.GetID());
        if (it != m_Resources.end()) {
            return it->second.Generation == handle.GetGeneration();
        }
        return false;
    }
    
    /**
     * @brief Get total number of registered resources
     */
    size_t GetCount() const {
        return m_Resources.size();
    }
    
    /**
     * @brief Clear all resources
     */
    void Clear() {
        m_Resources.clear();
    }
    
private:
    std::unordered_map<UUID, ResourceEntry<T>> m_Resources;
};

} // namespace MyEngine
