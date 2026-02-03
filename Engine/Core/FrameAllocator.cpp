/******************************************************************************
 * File: FrameAllocator.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Frame allocator implementation
 ******************************************************************************/

#include "FrameAllocator.h"

namespace MyEngine {

FrameAllocator::FrameAllocator(size_t sizePerFrame)
    : m_CurrentBuffer(0), m_FrameIndex(0) {
    
    // Create double buffers
    for (uint32_t i = 0; i < BUFFER_COUNT; ++i) {
        m_Buffers[i] = new LinearAllocator(sizePerFrame, MemoryTag::General);
    }
}

FrameAllocator::~FrameAllocator() {
    for (uint32_t i = 0; i < BUFFER_COUNT; ++i) {
        delete m_Buffers[i];
    }
}

void* FrameAllocator::Allocate(size_t size, size_t alignment) {
    return m_Buffers[m_CurrentBuffer]->Allocate(size, alignment);
}

void FrameAllocator::NextFrame() {
    // Switch to next buffer
    m_CurrentBuffer = (m_CurrentBuffer + 1) % BUFFER_COUNT;
    
    // Reset the buffer we just switched to (it's from 2 frames ago)
    m_Buffers[m_CurrentBuffer]->Reset();
    
    m_FrameIndex++;
}

} // namespace MyEngine
