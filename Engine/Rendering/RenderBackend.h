/******************************************************************************
 * File: RenderBackend.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Base class for Render Hardware Interface (RHI)
 ******************************************************************************/

#pragma once

#include "Math/MathTypes.h"
#include <memory>

namespace MyEngine {

enum class RendererAPI {
    None = 0,
    OpenGL = 1,
    Vulkan = 2
};

/**
 * @brief Abstract interface for rendering backends
 */
class RenderBackend {
public:
    virtual ~RenderBackend() = default;

    virtual void Init() = 0;
    virtual void Shutdown() = 0;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;

    virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
    virtual void SetClearColor(const Vec4& color) = 0;
    virtual void Clear() = 0;

    virtual void DrawIndexed(uint32_t indexCount) = 0;

    static RendererAPI GetAPI() { return s_API; }
    static std::unique_ptr<RenderBackend> Create();

private:
    static RendererAPI s_API;
};

} // namespace MyEngine
