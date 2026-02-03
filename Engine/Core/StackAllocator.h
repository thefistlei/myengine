/******************************************************************************
 * File: StackAllocator.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Stack allocator (LIFO) for temporary allocations
 ******************************************************************************/

#pragma once

#include "Allocator.h"
#include <cstdint>

namespace MyEngine {

/**
 * @brief Stack allocator with LIFO deallocation
 * 
 * Useful for scope-based temporary allocations.
 * Deallocation must follow LIFO order (stack discipline).
 */
class StackAllocator : public Allocator {
public:
    /**
     * @brief Constructor
     * @param size Total size in bytes
     * @param tag Memory tag for tracking
     */
    StackAllocator(size_t size, MemoryTag tag = MemoryTag::General);
    
    /**
     * @brief Destructor
     */
    ~StackAllocator();

    /**
     * @brief Allocate memory with alignment
     * @param size Size in bytes
     * @param alignment Alignment requirement (must be power of 2)
     * @return Pointer to allocated memory
     */
    void* Allocate(size_t size, size_t alignment = 8) override;

    /**
     * @brief Deallocate memory (must follow LIFO order)
     * @param ptr Pointer to deallocate
     */
    void Deallocate(void* ptr) override;

    /**
     * @brief Reset allocator (free all)
     */
    void Reset() override;

    /**
     * @brief Get current marker position
     */
    size_t GetMarker() const { return m_Offset; }

    /**
     * @brief Roll back to previous marker
     */
    void RollbackToMarker(size_t marker);

    /**
     * @brief Get used memory
     */
    size_t GetUsedMemory() const override { return m_Offset; }

    /**
     * @brief Get total memory
     */
    size_t GetTotalMemory() const override { return m_Size; }

private:
    void* m_Memory;
    size_t m_Size;
    size_t m_Offset;
    
    // Track allocation headers for validation
    struct AllocationHeader {
        size_t size;
        size_t padding;
    };
};

/**
 * @brief RAII marker for automatic rollback
 */
class StackAllocatorMarker {
public:
    StackAllocatorMarker(StackAllocator& allocator)
        : m_Allocator(allocator), m_Marker(allocator.GetMarker()) {}
    
    ~StackAllocatorMarker() {
        m_Allocator.RollbackToMarker(m_Marker);
    }

private:
    StackAllocator& m_Allocator;
    size_t m_Marker;
};

} // namespace MyEngine
