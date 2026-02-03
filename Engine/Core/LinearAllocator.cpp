/******************************************************************************
 * File: LinearAllocator.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Linear allocator implementation
 ******************************************************************************/

#include "LinearAllocator.h"
#include <cstring>

namespace MyEngine {

LinearAllocator::LinearAllocator(size_t size, MemoryTag tag)
    : Allocator(tag), m_Size(size), m_Offset(0) {
    m_Memory = std::malloc(size);
    assert(m_Memory && "Failed to allocate memory for LinearAllocator");
}

LinearAllocator::~LinearAllocator() {
    std::free(m_Memory);
}

void* LinearAllocator::Allocate(size_t size, size_t alignment) {
    // Calculate aligned offset
    size_t padding = 0;
    size_t currentAddress = reinterpret_cast<uintptr_t>(m_Memory) + m_Offset;
    size_t misalignment = currentAddress & (alignment - 1);
    
    if (misalignment != 0) {
        padding = alignment - misalignment;
    }
    
    // Check if we have enough space
    if (m_Offset + padding + size > m_Size) {
        assert(false && "LinearAllocator out of memory");
        return nullptr;
    }
    
    // Allocate
    m_Offset += padding;
    void* ptr = reinterpret_cast<char*>(m_Memory) + m_Offset;
    m_Offset += size;
    
    return ptr;
}

void LinearAllocator::Deallocate(void* ptr) {
    // No-op: linear allocator doesn't support individual deallocation
    // Must call Reset() to free all memory
}

void LinearAllocator::Reset() {
    m_Offset = 0;
}

} // namespace MyEngine
