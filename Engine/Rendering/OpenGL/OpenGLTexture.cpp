/******************************************************************************
 * File: OpenGLTexture.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: OpenGL texture implementation
 ******************************************************************************/

#include "OpenGLTexture.h"
#include "Core/Log.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace MyEngine {

Texture* Texture::Create(const TextureSpecification& spec) {
    return new OpenGLTexture(spec);
}

Texture* Texture::Create(const std::string& filepath) {
    return new OpenGLTexture(filepath);
}

OpenGLTexture::OpenGLTexture(const TextureSpecification& spec)
    : m_Specification(spec), m_Width(spec.Width), m_Height(spec.Height) {
    
    m_InternalFormat = GL_RGBA8;
    m_DataFormat = GL_RGBA;
    
    glGenTextures(1, &m_RendererID);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, nullptr);
}

OpenGLTexture::OpenGLTexture(const std::string& filepath)
    : m_Width(1), m_Height(1) {  // Initialize to 1x1 in case of failure
    
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    stbi_uc* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    
    if (data) {
        m_Width = width;
        m_Height = height;
        
        GLenum internalFormat = 0, dataFormat = 0;
        if (channels == 4) {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        } else if (channels == 3) {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        }
        
        m_InternalFormat = internalFormat;
        m_DataFormat = dataFormat;
        
        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        stbi_image_free(data);
        ENGINE_INFO("[OpenGLTexture] Successfully loaded: {} ({}x{}, {} channels)", filepath, width, height, channels);
    } else {
        ENGINE_ERROR("[OpenGLTexture] Failed to load texture: {}", filepath);
        
        // Create a 1x1 white texture as fallback
        m_InternalFormat = GL_RGBA8;
        m_DataFormat = GL_RGBA;
        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
        unsigned char whitePixel[4] = {255, 255, 255, 255};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    }
}

OpenGLTexture::~OpenGLTexture() {
    glDeleteTextures(1, &m_RendererID);
}

void OpenGLTexture::SetData(void* data, uint32_t size) {
    uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
    if (size != m_Width * m_Height * bpp) {
        ENGINE_ERROR("Data must be entire texture!");
        return;
    }
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
}

void OpenGLTexture::Bind(uint32_t slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
}

std::shared_ptr<Texture> TextureLoader::Load(const std::string& filepath) {
    return std::shared_ptr<Texture>(Texture::Create(filepath));
}

} // namespace MyEngine
