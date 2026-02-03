/******************************************************************************
 * File: Registry.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: ECS Registry (World) for managing entities and components
 ******************************************************************************/

#pragma once

#include <vector>
#include <queue>
#include <array>
#include <bitset>
#include <memory>
#include <unordered_map>
#include "Component.h"
#include "Core/Log.h"

namespace MyEngine {

using EntityID = uint32_t;
using Signature = std::bitset<MAX_COMPONENTS>;

/**
 * @brief Internal interface for component storage
 */
class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual void EntityDestroyed(EntityID entity) = 0;
};

/**
 * @brief Packed storage for components of a specific type
 */
template<typename T>
class ComponentArray : public IComponentArray {
public:
    void InsertData(EntityID entity, T component) {
        if (m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end()) {
            ENGINE_ERROR("Component added to same entity more than once.");
            return;
        }

        size_t newIndex = m_Size;
        m_EntityToIndexMap[entity] = newIndex;
        m_IndexToEntityMap[newIndex] = entity;
        m_ComponentArray[newIndex] = component;
        m_Size++;
    }

    void RemoveData(EntityID entity) {
        if (m_EntityToIndexMap.find(entity) == m_EntityToIndexMap.end()) {
            ENGINE_ERROR("Removing non-existent component.");
            return;
        }

        size_t indexOfRemovedEntity = m_EntityToIndexMap[entity];
        size_t indexOfLastElement = m_Size - 1;
        m_ComponentArray[indexOfRemovedEntity] = m_ComponentArray[indexOfLastElement];

        EntityID entityOfLastElement = m_IndexToEntityMap[indexOfLastElement];
        m_EntityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
        m_IndexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

        m_EntityToIndexMap.erase(entity);
        m_IndexToEntityMap.erase(indexOfLastElement);

        m_Size--;
    }

    T& GetData(EntityID entity) {
        if (m_EntityToIndexMap.find(entity) == m_EntityToIndexMap.end()) {
            ENGINE_ERROR("Retrieving non-existent component.");
            static T dummy;
            return dummy;
        }
        return m_ComponentArray[m_EntityToIndexMap[entity]];
    }

    void EntityDestroyed(EntityID entity) override {
        if (m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end()) {
            RemoveData(entity);
        }
    }

private:
    std::array<T, MAX_ENTITIES> m_ComponentArray;
    std::unordered_map<EntityID, size_t> m_EntityToIndexMap;
    std::unordered_map<size_t, EntityID> m_IndexToEntityMap;
    size_t m_Size = 0;
};

/**
 * @brief Core ECS manager
 */
class Registry {
public:
    Registry();
    
    EntityID CreateEntity();
    void DestroyEntity(EntityID entity);

    template<typename T>
    void RegisterComponent() {
        const char* typeName = typeid(T).name();
        if (m_ComponentTypes.find(typeName) != m_ComponentTypes.end()) {
            return;
        }
        m_ComponentTypes[typeName] = GetComponentTypeID<T>();
        m_ComponentArrays[typeName] = std::make_shared<ComponentArray<T>>();
    }

    template<typename T>
    void AddComponent(EntityID entity, T component) {
        GetComponentArray<T>()->InsertData(entity, component);
        auto type = GetComponentType<T>();
        m_Signatures[entity].set(type);
    }

    template<typename T>
    void RemoveComponent(EntityID entity) {
        GetComponentArray<T>()->RemoveData(entity);
        auto type = GetComponentType<T>();
        m_Signatures[entity].reset(type);
    }

    template<typename T>
    T& GetComponent(EntityID entity) {
        return GetComponentArray<T>()->GetData(entity);
    }

    template<typename T>
    ComponentID GetComponentType() {
        const char* typeName = typeid(T).name();
        return m_ComponentTypes[typeName];
    }

    Signature GetSignature(EntityID entity) {
        return m_Signatures[entity];
    }

    template<typename... Components>
    std::vector<EntityID> GetEntitiesWith() {
        std::vector<EntityID> entities;
        Signature targetSignature;
        ComponentID componentIDs[] = { GetComponentType<Components>()... };
        for (ComponentID id : componentIDs) {
            targetSignature.set(id);
        }

        for (EntityID i = 0; i < MAX_ENTITIES; i++) {
            if (m_ActiveEntities.test(i) && (m_Signatures[i] & targetSignature) == targetSignature) {
                entities.push_back(i);
            }
        }
        return entities;
    }

private:
    template<typename T>
    std::shared_ptr<ComponentArray<T>> GetComponentArray() {
        const char* typeName = typeid(T).name();
        return std::static_pointer_cast<ComponentArray<T>>(m_ComponentArrays[typeName]);
    }

private:
    std::queue<EntityID> m_AvailableEntities;
    std::bitset<MAX_ENTITIES> m_ActiveEntities;
    std::array<Signature, MAX_ENTITIES> m_Signatures;
    uint32_t m_LivingEntityCount = 0;

    std::unordered_map<const char*, ComponentID> m_ComponentTypes;
    std::unordered_map<const char*, std::shared_ptr<IComponentArray>> m_ComponentArrays;
};

} // namespace MyEngine
