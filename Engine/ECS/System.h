/******************************************************************************
 * File: System.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: ECS System base class
 ******************************************************************************/

#pragma once

#include <set>
#include "Registry.h"

namespace MyEngine {

/**
 * @brief Base class for all ECS systems
 */
class System {
public:
    std::set<EntityID> m_Entities;
};

/**
 * @brief Manager for ECS systems
 */
class SystemManager {
public:
    template<typename T>
    std::shared_ptr<T> RegisterSystem(Registry* registry) {
        const char* typeName = typeid(T).name();
        auto system = std::make_shared<T>();
        m_Systems[typeName] = system;
        return system;
    }

    template<typename T>
    void SetSignature(Signature signature) {
        const char* typeName = typeid(T).name();
        m_Signatures[typeName] = signature;
    }

    void EntityDestroyed(EntityID entity) {
        for (auto const& pair : m_Systems) {
            auto const& system = pair.second;
            system->m_Entities.erase(entity);
        }
    }

    void EntitySignatureChanged(EntityID entity, Signature entitySignature) {
        for (auto const& pair : m_Systems) {
            auto const& type = pair.first;
            auto const& system = pair.second;
            auto const& systemSignature = m_Signatures[type];

            if ((entitySignature & systemSignature) == systemSignature) {
                system->m_Entities.insert(entity);
            } else {
                system->m_Entities.erase(entity);
            }
        }
    }

private:
    std::unordered_map<const char*, Signature> m_Signatures;
    std::unordered_map<const char*, std::shared_ptr<System>> m_Systems;
};

} // namespace MyEngine
