/******************************************************************************
 * File: Allocator.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Memory allocator interface and memory tags
 * Dependencies: <cstddef>, <cstdint>
 ******************************************************************************/

#pragma once

#include <cstddef>
#include <cstdint>

namespace MyEngine {

/**
 * @brief Memory allocation tags for tracking
 */
enum class MemoryTag {
    Unknown = 0,
    General,
    Rendering,
    Physics,
    Audio,
    Scripting,
    ECS,
    Resource,
    Platform,
    Core,
    COUNT
};

/**
 * @brief Base allocator interface
 */
class Allocator {
public:
    explicit Allocator(MemoryTag tag = MemoryTag::General) : m_Tag(tag) {}
    virtual ~Allocator() = default;

    /**
     * @brief Allocate memory with alignment
     * @param size Size in bytes
     * @param alignment Alignment requirement (default 8)
     * @return Pointer to allocated memory
     */
    virtual void* Allocate(size_t size, size_t alignment = 8) = 0;

    /**
     * @brief Deallocate memory
     * @param ptr Pointer to deallocate
     */
    virtual void Deallocate(void* ptr) = 0;

    /**
     * @brief Reset allocator (free all memory)
     */
    virtual void Reset() = 0;

    /**
     * @brief Get currently used memory in bytes
     */
    virtual size_t GetUsedMemory() const = 0;

    /**
     * @brief Get total capacity in bytes
     */
    virtual size_t GetTotalMemory() const = 0;

protected:
    MemoryTag m_Tag;
};

} // namespace MyEngine
