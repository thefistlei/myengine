/******************************************************************************
 * File: LinearAllocator.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Linear (bump) allocator - fast allocation, reset only
 * Dependencies: Allocator.h
 ******************************************************************************/

#pragma once

#include "Allocator.h"
#include <cstdlib>
#include <cassert>

namespace MyEngine {

/**
 * @brief Linear allocator (bump allocator)
 * - Very fast allocation (just bump pointer)
 * - No individual deallocation
 * - Must reset to free all memory
 * - Good for per-frame allocations
 */
class LinearAllocator : public Allocator {
public:
    explicit LinearAllocator(size_t size, MemoryTag tag = MemoryTag::General);
    ~LinearAllocator() override;

    void* Allocate(size_t size, size_t alignment = 8) override;
    void Deallocate(void* ptr) override; // No-op
    void Reset() override;

    size_t GetUsedMemory() const override { return m_Offset; }
    size_t GetTotalMemory() const override { return m_Size; }

private:
    void* m_Memory;
    size_t m_Size;
    size_t m_Offset;
};

} // namespace MyEngine
