/******************************************************************************
 * File: OpenGLRenderBackend.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: OpenGL implementation of RenderBackend
 ******************************************************************************/

#include "OpenGLRenderBackend.h"
#include <glad/gl.h>

namespace MyEngine {

void OpenGLRenderBackend::Init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void OpenGLRenderBackend::Shutdown() {
    // No specific shutdown needed for basic OpenGL backend for now
}

void OpenGLRenderBackend::BeginFrame() {
}

void OpenGLRenderBackend::EndFrame() {
    // Swap buffers is usually handled by the window/context, 
    // but in some RHI designs it might be triggered here.
}

void OpenGLRenderBackend::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    glViewport(x, y, width, height);
}

void OpenGLRenderBackend::SetClearColor(const Vec4& color) {
    glClearColor(color.x, color.y, color.z, color.w);
}

void OpenGLRenderBackend::Clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderBackend::DrawIndexed(uint32_t indexCount) {
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
}

} // namespace MyEngine
