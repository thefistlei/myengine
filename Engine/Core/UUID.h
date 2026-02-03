/******************************************************************************
 * File: UUID.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: UUID (Universally Unique Identifier) generator
 * Dependencies: <cstdint>, <string>
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <string>

namespace MyEngine {

/**
 * @brief UUID class for generating and managing unique identifiers
 * 
 * Uses 64-bit integer for simplicity. Can be extended to full 128-bit UUID later.
 * Provides fast generation and comparison operations.
 */
class UUID {
public:
    UUID();                          // Generate new UUID
    explicit UUID(uint64_t id);      // Create from existing ID
    
    uint64_t Get() const { return m_UUID; }
    std::string ToString() const;
    
    bool operator==(const UUID& other) const { return m_UUID == other.m_UUID; }
    bool operator!=(const UUID& other) const { return m_UUID != other.m_UUID; }
    bool operator<(const UUID& other) const { return m_UUID < other.m_UUID; }
    
private:
    uint64_t m_UUID;
};

} // namespace MyEngine

// std::hash specialization for UUID
namespace std {
    template<>
    struct hash<MyEngine::UUID> {
        size_t operator()(const MyEngine::UUID& uuid) const noexcept {
            return hash<uint64_t>()(uuid.Get());
        }
    };
} // namespace std
