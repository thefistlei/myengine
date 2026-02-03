/******************************************************************************
 * File: OpenGLRenderBackend.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: OpenGL implementation of RenderBackend
 ******************************************************************************/

#pragma once

#include "Rendering/RenderBackend.h"

namespace MyEngine {

class OpenGLRenderBackend : public RenderBackend {
public:
    virtual void Init() override;
    virtual void Shutdown() override;

    virtual void BeginFrame() override;
    virtual void EndFrame() override;

    virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
    virtual void SetClearColor(const Vec4& color) override;
    virtual void Clear() override;

    virtual void DrawIndexed(uint32_t indexCount) override;
};

} // namespace MyEngine
