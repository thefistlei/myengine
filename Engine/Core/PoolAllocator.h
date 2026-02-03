/******************************************************************************
 * File: PoolAllocator.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Pool allocator for fixed-size objects
 ******************************************************************************/

#pragma once

#include "Allocator.h"
#include <cstdint>

namespace MyEngine {

/**
 * @brief Pool allocator for fixed-size allocations
 * 
 * Efficient for allocating many objects of the same size.
 * Uses a free list for O(1) allocation and deallocation.
 */
class PoolAllocator : public Allocator {
public:
    /**
     * @brief Constructor
     * @param objectSize Size of each object in bytes
     * @param objectCount Number of objects in pool
     * @param tag Memory tag for tracking
     */
    PoolAllocator(size_t objectSize, size_t objectCount, MemoryTag tag = MemoryTag::General);
    
    /**
     * @brief Destructor
     */
    ~PoolAllocator();

    /**
     * @brief Allocate one object
     */
    void* Allocate(size_t size, size_t alignment = 8) override;

    /**
     * @brief Deallocate one object
     */
    void Deallocate(void* ptr) override;

    /**
     * @brief Reset allocator (mark all free)
     */
    void Reset() override;

    /**
     * @brief Get object size
     */
    size_t GetObjectSize() const { return m_ObjectSize; }

    /**
     * @brief Get number of allocated objects
     */
    size_t GetAllocatedCount() const { return m_AllocatedCount; }

    /**
     * @brief Get total capacity
     */
    size_t GetCapacity() const { return m_ObjectCount; }

    /**
     * @brief Get used memory
     */
    size_t GetUsedMemory() const override { return m_AllocatedCount * m_ObjectSize; }

    /**
     * @brief Get total memory
     */
    size_t GetTotalMemory() const override { return m_ObjectCount * m_ObjectSize; }

private:
    void* m_Memory;
    size_t m_ObjectSize;
    size_t m_ObjectCount;
    size_t m_AllocatedCount;
    
    // Free list node
    struct FreeNode {
        FreeNode* next;
    };
    
    FreeNode* m_FreeList;
    
    void InitializeFreeList();
};

} // namespace MyEngine
