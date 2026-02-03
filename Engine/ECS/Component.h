/******************************************************************************
 * File: Component.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: ECS Component base and traits
 ******************************************************************************/

#pragma once

#include <cstdint>

namespace MyEngine {

/**
 * @brief Base type for component IDs
 */
using ComponentID = uint32_t;

/**
 * @brief Internal counter for generating unique component IDs
 */
namespace Internal {
    inline ComponentID GetUniqueComponentID() {
        static ComponentID lastID = 0;
        return lastID++;
    }
}

/**
 * @brief Template to get a unique ID for each component type
 */
template<typename T>
inline ComponentID GetComponentTypeID() {
    static ComponentID typeID = Internal::GetUniqueComponentID();
    return typeID;
}

/**
 * @brief Maximum number of entities supported
 */
const uint32_t MAX_ENTITIES = 10000;

/**
 * @brief Maximum number of components supported
 */
const uint32_t MAX_COMPONENTS = 64;

} // namespace MyEngine
