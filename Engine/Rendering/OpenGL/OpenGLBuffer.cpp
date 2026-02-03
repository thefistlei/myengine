/******************************************************************************
 * File: OpenGLBuffer.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: OpenGL implementation of Vertex and Index buffers
 ******************************************************************************/

#include "OpenGLBuffer.h"
#include <glad/gl.h>

namespace MyEngine {

//////////////////////////////////////////////////////////////////////////////////
// VertexBuffer //////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size) {
    return new OpenGLVertexBuffer(vertices, size);
}

VertexBuffer* VertexBuffer::Create(uint32_t size) {
    return new OpenGLVertexBuffer(size);
}

OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size) {
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size) {
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}

OpenGLVertexBuffer::~OpenGLVertexBuffer() {
    glDeleteBuffers(1, &m_RendererID);
}

void OpenGLVertexBuffer::Bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
}

void OpenGLVertexBuffer::Unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OpenGLVertexBuffer::SetData(const void* data, uint32_t size) {
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

//////////////////////////////////////////////////////////////////////////////////
// IndexBuffer ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count) {
    return new OpenGLIndexBuffer(indices, count);
}

OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
    : m_Count(count) {
    glGenBuffers(1, &m_RendererID);
    
    // GL_ELEMENT_ARRAY_BUFFER is not valid without an active VAO
    // Binding here because OpenGLIndexBuffer is usually bound with a VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
}

OpenGLIndexBuffer::~OpenGLIndexBuffer() {
    glDeleteBuffers(1, &m_RendererID);
}

void OpenGLIndexBuffer::Bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
}

void OpenGLIndexBuffer::Unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

} // namespace MyEngine
