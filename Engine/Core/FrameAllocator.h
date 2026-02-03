/******************************************************************************
 * File: FrameAllocator.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Frame allocator that resets every frame
 ******************************************************************************/

#pragma once

#include "LinearAllocator.h"

namespace MyEngine {

/**
 * @brief Frame allocator - resets every frame
 * 
 * Uses double buffering to avoid stalls:
 * - Frame N allocates from buffer A
 * - Frame N+1 allocates from buffer B (while N is rendering)
 * - Frame N+2 resets buffer A and uses it again
 */
class FrameAllocator {
public:
    /**
     * @brief Constructor
     * @param sizePerFrame Size per frame buffer
     */
    FrameAllocator(size_t sizePerFrame);
    
    /**
     * @brief Destructor
     */
    ~FrameAllocator();

    /**
     * @brief Allocate memory for current frame
     */
    void* Allocate(size_t size, size_t alignment = 8);

    /**
     * @brief Switch to next frame (resets old buffer)
     */
    void NextFrame();

    /**
     * @brief Get current frame index
     */
    uint32_t GetFrameIndex() const { return m_FrameIndex; }

private:
    static constexpr uint32_t BUFFER_COUNT = 2;
    
    LinearAllocator* m_Buffers[BUFFER_COUNT];
    uint32_t m_CurrentBuffer;
    uint32_t m_FrameIndex;
};

} // namespace MyEngine
