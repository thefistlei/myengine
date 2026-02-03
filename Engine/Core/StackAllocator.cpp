/******************************************************************************
 * File: StackAllocator.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Stack allocator implementation
 ******************************************************************************/

#include "StackAllocator.h"
#include "MemoryTracker.h"
#include <cassert>
#include <cstdlib>
#include <cstring>

namespace MyEngine {

StackAllocator::StackAllocator(size_t size, MemoryTag tag)
    : Allocator(tag), m_Size(size), m_Offset(0) {
    m_Memory = std::malloc(size);
    assert(m_Memory && "Failed to allocate stack memory");
    
    MemoryTracker::RecordAllocation(size, tag);
}

StackAllocator::~StackAllocator() {
    MemoryTracker::RecordDeallocation(m_Size, m_Tag);
    std::free(m_Memory);
}

void* StackAllocator::Allocate(size_t size, size_t alignment) {
    // Calculate padding for alignment
    size_t headerSize = sizeof(AllocationHeader);
    size_t currentAddress = reinterpret_cast<uintptr_t>(m_Memory) + m_Offset;
    size_t alignedAddress = (currentAddress + headerSize + alignment - 1) & ~(alignment - 1);
    size_t padding = alignedAddress - currentAddress;

    // Check if we have enough space
    if (m_Offset + padding + size > m_Size) {
        assert(false && "StackAllocator out of memory");
        return nullptr;
    }

    // Store header
    AllocationHeader* header = reinterpret_cast<AllocationHeader*>(
        reinterpret_cast<char*>(m_Memory) + m_Offset
    );
    header->size = size;
    header->padding = padding;

    // Allocate
    m_Offset += padding + size;
    void* ptr = reinterpret_cast<void*>(alignedAddress);

    return ptr;
}

void StackAllocator::Deallocate(void* ptr) {
    if (!ptr) return;

    // Calculate header position
    uintptr_t ptrAddress = reinterpret_cast<uintptr_t>(ptr);
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(m_Memory);

    // Find header before this allocation
    size_t offset = ptrAddress - memoryAddress;
    
    // In a proper stack allocator, we should validate LIFO order
    // For now, we just move back the offset
    AllocationHeader* header = reinterpret_cast<AllocationHeader*>(
        reinterpret_cast<char*>(m_Memory) + offset - sizeof(AllocationHeader)
    );

    m_Offset = offset - header->padding;
}

void StackAllocator::Reset() {
    m_Offset = 0;
}

void StackAllocator::RollbackToMarker(size_t marker) {
    assert(marker <= m_Offset && "Invalid marker");
    m_Offset = marker;
}

} // namespace MyEngine
