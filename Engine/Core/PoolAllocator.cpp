/******************************************************************************
 * File: PoolAllocator.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Pool allocator implementation
 ******************************************************************************/

#include "PoolAllocator.h"
#include "MemoryTracker.h"
#include <cassert>
#include <cstdlib>
#include <algorithm>

namespace MyEngine {

PoolAllocator::PoolAllocator(size_t objectSize, size_t objectCount, MemoryTag tag)
    : Allocator(tag)
    , m_ObjectSize(std::max(objectSize, sizeof(FreeNode)))  // Ensure size can fit FreeNode
    , m_ObjectCount(objectCount)
    , m_AllocatedCount(0)
    , m_FreeList(nullptr) {
    
    size_t totalSize = m_ObjectSize * m_ObjectCount;
    m_Memory = std::malloc(totalSize);
    assert(m_Memory && "Failed to allocate pool memory");
    
    MemoryTracker::RecordAllocation(totalSize, tag);
    
    InitializeFreeList();
}

PoolAllocator::~PoolAllocator() {
    size_t totalSize = m_ObjectSize * m_ObjectCount;
    MemoryTracker::RecordDeallocation(totalSize, m_Tag);
    std::free(m_Memory);
}

void PoolAllocator::InitializeFreeList() {
    // Link all objects into free list
    char* current = static_cast<char*>(m_Memory);
    m_FreeList = reinterpret_cast<FreeNode*>(current);
    
    FreeNode* node = m_FreeList;
    for (size_t i = 0; i < m_ObjectCount - 1; ++i) {
        FreeNode* next = reinterpret_cast<FreeNode*>(current + m_ObjectSize);
        node->next = next;
        node = next;
        current += m_ObjectSize;
    }
    
    node->next = nullptr;  // Last node
}

void* PoolAllocator::Allocate(size_t size, size_t alignment) {
    // Pool allocator ignores size parameter (fixed size)
    (void)size;
    (void)alignment;
    
    if (!m_FreeList) {
        assert(false && "PoolAllocator exhausted");
        return nullptr;
    }
    
    // Pop from free list
    void* ptr = m_FreeList;
    m_FreeList = m_FreeList->next;
    m_AllocatedCount++;
    
    return ptr;
}

void PoolAllocator::Deallocate(void* ptr) {
    if (!ptr) return;
    
    // Push to free list
    FreeNode* node = static_cast<FreeNode*>(ptr);
    node->next = m_FreeList;
    m_FreeList = node;
    m_AllocatedCount--;
}

void PoolAllocator::Reset() {
    m_AllocatedCount = 0;
    InitializeFreeList();
}

} // namespace MyEngine
