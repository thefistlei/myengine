/******************************************************************************
 * File: Texture.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Texture resource abstraction
 ******************************************************************************/

#pragma once

#include <string>
#include <cstdint>
#include <memory>

namespace MyEngine {

/**
 * @brief Texture formats
 */
enum class TextureFormat {
    None = 0,
    RGB8, RGBA8,
    R16F, RG16F, RGB16F, RGBA16F,
    R32F, RG32F, RGB32F, RGBA32F
};

/**
 * @brief Texture filtering modes
 */
enum class TextureFilter {
    Nearest, Linear
};

/**
 * @brief Texture wrap modes
 */
enum class TextureWrap {
    Repeat, ClampToEdge, ClampToBorder
};

/**
 * @brief Configuration for creating a texture
 */
struct TextureSpecification {
    uint32_t Width = 1;
    uint32_t Height = 1;
    TextureFormat Format = TextureFormat::RGBA8;
    TextureFilter MinFilter = TextureFilter::Linear;
    TextureFilter MagFilter = TextureFilter::Linear;
    TextureWrap WrapS = TextureWrap::Repeat;
    TextureWrap WrapT = TextureWrap::Repeat;
    bool GenerateMips = true;
};

/**
 * @brief Texture interface
 */
class Texture {
public:
    virtual ~Texture() = default;
    
    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual uint32_t GetRendererID() const = 0;
    
    virtual void SetData(void* data, uint32_t size) = 0;
    virtual void Bind(uint32_t slot = 0) const = 0;
    
    static Texture* Create(const TextureSpecification& spec);
    static Texture* Create(const std::string& filepath);
};

/**
 * @brief Static utility for loading textures
 */
class TextureLoader {
public:
    static std::shared_ptr<Texture> Load(const std::string& filepath);
};

} // namespace MyEngine
