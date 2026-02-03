/******************************************************************************
 * File: Entity.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Entity wrapper class
 ******************************************************************************/

#pragma once

#include "Registry.h"

namespace MyEngine {

/**
 * @brief High-level entity wrapper
 */
class Entity {
public:
    Entity() = default;
    Entity(EntityID handle, Registry* registry) : m_EntityHandle(handle), m_Registry(registry) {}
    
    template<typename T, typename... Args>
    T& AddComponent(Args&&... args) {
        m_Registry->AddComponent<T>(m_EntityHandle, T(std::forward<Args>(args)...));
        return m_Registry->GetComponent<T>(m_EntityHandle);
    }
    
    template<typename T>
    void RemoveComponent() {
        m_Registry->RemoveComponent<T>(m_EntityHandle);
    }
    
    template<typename T>
    T& GetComponent() {
        return m_Registry->GetComponent<T>(m_EntityHandle);
    }
    
    template<typename T>
    bool HasComponent() {
        auto signature = m_Registry->GetSignature(m_EntityHandle);
        auto type = m_Registry->GetComponentType<T>();
        return signature.test(type);
    }
    
    operator EntityID() const { return m_EntityHandle; }
    operator bool() const { return m_EntityHandle != 0 && m_Registry != nullptr; }
    
    bool operator==(const Entity& other) const {
        return m_EntityHandle == other.m_EntityHandle && m_Registry == other.m_Registry;
    }
    
private:
    EntityID m_EntityHandle = 0;
    Registry* m_Registry = nullptr;
};

} // namespace MyEngine
