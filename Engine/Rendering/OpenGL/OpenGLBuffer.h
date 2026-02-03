/******************************************************************************
 * File: OpenGLBuffer.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: OpenGL implementation of Vertex and Index buffers
 ******************************************************************************/

#pragma once

#include "Rendering/Buffer.h"

namespace MyEngine {

/**
 * @brief OpenGL implementation of the vertex buffer
 */
class OpenGLVertexBuffer : public VertexBuffer {
public:
    OpenGLVertexBuffer(float* vertices, uint32_t size);
    OpenGLVertexBuffer(uint32_t size);
    virtual ~OpenGLVertexBuffer();
    
    void Bind() const override;
    void Unbind() const override;
    
    void SetData(const void* data, uint32_t size) override;
    
    const BufferLayout& GetLayout() const override { return m_Layout; }
    void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
    
private:
    uint32_t m_RendererID;
    BufferLayout m_Layout;
};

/**
 * @brief OpenGL implementation of the index buffer
 */
class OpenGLIndexBuffer : public IndexBuffer {
public:
    OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
    virtual ~OpenGLIndexBuffer();
    
    void Bind() const override;
    void Unbind() const override;
    
    uint32_t GetCount() const override { return m_Count; }
    
private:
    uint32_t m_RendererID;
    uint32_t m_Count;
};

} // namespace MyEngine
