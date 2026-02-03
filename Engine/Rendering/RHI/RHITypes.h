/******************************************************************************
 * File: RHITypes.h
 * Author: AI Assistant
 * Created: 2026-01-31
 * Description: RHI Core Types - Type-safe resource handles and descriptors
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace MyEngine {

// ============================================================================
// Backend Type
// ============================================================================

enum class BackendType {
    OpenGL,
    Vulkan,
    D3D12,      // Future
    Metal       // Future
};

// ============================================================================
// Type-safe Resource Handles
// ============================================================================

template<typename T>
struct Handle {
    uint32_t index = UINT32_MAX;
    uint32_t generation = 0;
    
    bool IsValid() const { return index != UINT32_MAX; }
    void Invalidate() { index = UINT32_MAX; }
    
    bool operator==(const Handle& other) const {
        return index == other.index && generation == other.generation;
    }
    
    bool operator!=(const Handle& other) const {
        return !(*this == other);
    }
};

// Tag types for type safety
struct Buffer_Tag {};
struct Texture_Tag {};
struct Shader_Tag {};
struct Pipeline_Tag {};
struct RenderPass_Tag {};
struct Framebuffer_Tag {};
struct Sampler_Tag {};
struct DescriptorSet_Tag {};

// Typed handles
using BufferHandle = Handle<Buffer_Tag>;
using TextureHandle = Handle<Texture_Tag>;
using ShaderHandle = Handle<Shader_Tag>;
using PipelineHandle = Handle<Pipeline_Tag>;
using RenderPassHandle = Handle<RenderPass_Tag>;
using FramebufferHandle = Handle<Framebuffer_Tag>;
using SamplerHandle = Handle<Sampler_Tag>;
using DescriptorSetHandle = Handle<DescriptorSet_Tag>;

// ============================================================================
// Buffer Descriptors
// ============================================================================

enum class BufferUsage : uint32_t {
    None        = 0,
    Vertex      = 1 << 0,
    Index       = 1 << 1,
    Uniform     = 1 << 2,
    Storage     = 1 << 3,
    Indirect    = 1 << 4,
    TransferSrc = 1 << 5,
    TransferDst = 1 << 6
};

inline BufferUsage operator|(BufferUsage a, BufferUsage b) {
    return static_cast<BufferUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline BufferUsage operator&(BufferUsage a, BufferUsage b) {
    return static_cast<BufferUsage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline bool HasFlag(BufferUsage value, BufferUsage flag) {
    return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

enum class MemoryUsage {
    GPU_Only,       // Device local memory
    CPU_To_GPU,     // Upload heap (dynamic data)
    GPU_To_CPU,     // Readback heap (query results)
    CPU_Only        // Staging buffer
};

struct BufferDesc {
    uint64_t size = 0;
    BufferUsage usage = BufferUsage::None;
    MemoryUsage memoryUsage = MemoryUsage::GPU_Only;
    const char* debugName = nullptr;
};

// ============================================================================
// Texture Descriptors
// ============================================================================

enum class TextureFormat {
    // Color formats
    R8_UNORM, RG8_UNORM, RGBA8_UNORM, RGBA8_SRGB,
    R16F, RG16F, RGBA16F,
    R32F, RG32F, RGB32F, RGBA32F,
    
    // Depth/Stencil formats
    D16_UNORM, D24_UNORM_S8_UINT, D32F, D32F_S8_UINT,
    
    // Compressed formats
    BC1_RGBA, BC3_RGBA, BC5_RG, BC7_RGBA,
    
    // HDR formats
    R11G11B10F, RGB9E5
};

enum class TextureUsage : uint32_t {
    None           = 0,
    Sampled        = 1 << 0,  // Shader sampling
    Storage        = 1 << 1,  // UAV / Image
    RenderTarget   = 1 << 2,  // Color attachment
    DepthStencil   = 1 << 3,  // Depth/Stencil attachment
    TransferSrc    = 1 << 4,
    TransferDst    = 1 << 5
};

inline TextureUsage operator|(TextureUsage a, TextureUsage b) {
    return static_cast<TextureUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline TextureUsage operator&(TextureUsage a, TextureUsage b) {
    return static_cast<TextureUsage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline bool HasFlag(TextureUsage value, TextureUsage flag) {
    return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

struct TextureDesc {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 1;
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
    TextureFormat format = TextureFormat::RGBA8_UNORM;
    TextureUsage usage = TextureUsage::Sampled;
    bool isCubemap = false;
    const char* debugName = nullptr;
};

// ============================================================================
// Shader Descriptors
// ============================================================================

enum class ShaderStage : uint32_t {
    Vertex   = 1 << 0,
    Fragment = 1 << 1,
    Compute  = 1 << 2,
    Geometry = 1 << 3,
    TessControl = 1 << 4,
    TessEval = 1 << 5
};

inline ShaderStage operator|(ShaderStage a, ShaderStage b) {
    return static_cast<ShaderStage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

struct ShaderDesc {
    ShaderStage stage = ShaderStage::Vertex;
    const char* source = nullptr;
    const char* entryPoint = "main";
    const char* debugName = nullptr;
};

// ============================================================================
// Pipeline State Descriptors
// ============================================================================

enum class CompareOp {
    Never, Less, Equal, LessOrEqual,
    Greater, NotEqual, GreaterOrEqual, Always
};

enum class PrimitiveTopology {
    PointList, LineList, LineStrip,
    TriangleList, TriangleStrip, PatchList
};

enum class CullMode {
    None, Front, Back
};

enum class BlendMode {
    Opaque,          // No blending
    AlphaBlend,      // Standard alpha blend
    Additive,        // Additive blend
    Multiply,        // Multiply blend
    Custom           // Custom blend
};

struct RasterState {
    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    CullMode cullMode = CullMode::Back;
    bool frontFaceCCW = true;
    bool depthClampEnable = false;
    float depthBias = 0.0f;
    float depthBiasSlope = 0.0f;
};

struct DepthStencilState {
    bool depthTestEnable = true;
    bool depthWriteEnable = true;
    CompareOp depthCompareOp = CompareOp::Less;
    bool stencilTestEnable = false;
};

struct BlendState {
    bool blendEnable = false;
    BlendMode mode = BlendMode::Opaque;
};

// Forward declarations for vertex attributes
struct VertexAttribute;

struct PipelineDesc {
    ShaderHandle vertexShader;
    ShaderHandle fragmentShader;
    ShaderHandle geometryShader;      // Optional
    ShaderHandle tessControlShader;   // Optional
    ShaderHandle tessEvalShader;      // Optional
    
    RasterState rasterState;
    DepthStencilState depthStencilState;
    BlendState blendState;
    
    // Vertex input layout
    std::vector<VertexAttribute> vertexAttributes;
    
    // Render Pass compatibility
    RenderPassHandle renderPass;
    uint32_t subpassIndex = 0;
    
    const char* debugName = nullptr;
};

// ============================================================================
// Queue Types
// ============================================================================

enum class QueueType {
    Graphics,
    Compute,
    Transfer
};

} // namespace MyEngine
