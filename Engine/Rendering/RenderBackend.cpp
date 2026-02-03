/******************************************************************************
 * File: RenderBackend.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: RenderBackend static implementation
 ******************************************************************************/

#include "RenderBackend.h"
#include "OpenGL/OpenGLRenderBackend.h"

namespace MyEngine {

RendererAPI RenderBackend::s_API = RendererAPI::OpenGL;

std::unique_ptr<RenderBackend> RenderBackend::Create() {
    switch (s_API) {
        case RendererAPI::None: return nullptr;
        case RendererAPI::OpenGL: return std::make_unique<OpenGLRenderBackend>();
        case RendererAPI::Vulkan: return nullptr; // Not implemented
    }
    return nullptr;
}

} // namespace MyEngine
