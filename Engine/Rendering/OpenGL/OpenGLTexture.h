/******************************************************************************
 * File: OpenGLTexture.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: OpenGL texture implementation
 ******************************************************************************/

#pragma once

#include "Resource/Texture.h"
#include <glad/gl.h>

namespace MyEngine {

class OpenGLTexture : public Texture {
public:
    OpenGLTexture(const TextureSpecification& spec);
    OpenGLTexture(const std::string& filepath);
    virtual ~OpenGLTexture();
    
    uint32_t GetWidth() const override { return m_Width; }
    uint32_t GetHeight() const override { return m_Height; }
    uint32_t GetRendererID() const override { return m_RendererID; }
    
    void SetData(void* data, uint32_t size) override;
    void Bind(uint32_t slot = 0) const override;
    
private:
    TextureSpecification m_Specification;
    uint32_t m_Width, m_Height;
    uint32_t m_RendererID;
    GLenum m_InternalFormat, m_DataFormat;
};

} // namespace MyEngine
