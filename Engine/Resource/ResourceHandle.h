/******************************************************************************
 * File: ResourceHandle.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Type-safe resource handle system
 ******************************************************************************/

#pragma once

#include "Core/UUID.h"

namespace MyEngine {

/**
 * @brief Type-safe handle for resources
 */
template<typename T>
class Handle {
public:
    Handle() : m_ID(0), m_Generation(0) {}
    Handle(UUID id, uint32_t gen = 0) : m_ID(id), m_Generation(gen) {}
    
    UUID GetID() const { return m_ID; }
    uint32_t GetGeneration() const { return m_Generation; }
    
    bool IsValid() const { return m_ID.Get() != 0; }
    
    bool operator==(const Handle& other) const {
        return m_ID == other.m_ID && m_Generation == other.m_Generation;
    }
    
    bool operator!=(const Handle& other) const {
        return !(*this == other);
    }
    
private:
    UUID m_ID;
    uint32_t m_Generation;
};

} // namespace MyEngine

// Hash specialization for Handle
namespace std {
    template<typename T>
    struct hash<MyEngine::Handle<T>> {
        size_t operator()(const MyEngine::Handle<T>& handle) const {
            return hash<uint64_t>()(handle.GetID().Get()) ^ (hash<uint32_t>()(handle.GetGeneration()) << 1);
        }
    };
}
