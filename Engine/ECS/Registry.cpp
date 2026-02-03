/******************************************************************************
 * File: Registry.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Registry implementation
 ******************************************************************************/

#include "Registry.h"

namespace MyEngine {

Registry::Registry() {
    for (EntityID entity = 0; entity < MAX_ENTITIES; ++entity) {
        m_AvailableEntities.push(entity);
    }
}

EntityID Registry::CreateEntity() {
    if (m_LivingEntityCount >= MAX_ENTITIES) {
        ENGINE_ERROR("Too many entities in existence.");
        return 0;
    }

    EntityID id = m_AvailableEntities.front();
    m_AvailableEntities.pop();
    m_LivingEntityCount++;
    m_ActiveEntities.set(id);

    return id;
}

void Registry::DestroyEntity(EntityID entity) {
    if (entity >= MAX_ENTITIES) {
        ENGINE_ERROR("Entity out of range.");
        return;
    }

    m_Signatures[entity].reset();
    m_ActiveEntities.reset(entity);

    for (auto const& pair : m_ComponentArrays) {
        auto const& componentArray = pair.second;
        componentArray->EntityDestroyed(entity);
    }

    m_AvailableEntities.push(entity);
    m_LivingEntityCount--;
}

} // namespace MyEngine
