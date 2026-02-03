/******************************************************************************
 * File: RenderPass.cpp
 * Author: AI Assistant
 * Created: 2026-01-31
 * Description: Implementation of Pass base classes
 ******************************************************************************/

#include "RenderPass.h"
#include "Rendering/RenderBackend.h"
#include "Core/Log.h"
#include <glad/gl.h>
#include <algorithm>

namespace MyEngine {

// ============================================================================
// GraphicsPass Implementation
// ============================================================================

void GraphicsPass::ClearColor(const Vec4& color) {
    if (m_Backend) {
        m_Backend->SetClearColor(color);
        m_Backend->Clear();
    }
}

void GraphicsPass::ClearDepth(float depth) {
    // OpenGL-specific for now
    // In a full RHI, this would go through backend
    glClear(GL_DEPTH_BUFFER_BIT);
}

// ============================================================================
// PassManager Implementation
// ============================================================================

void PassManager::AddPass(std::unique_ptr<RenderPass> pass) {
    if (pass) {
        m_Passes.push_back(std::move(pass));
        SortPassesByPriority();
    }
}

void PassManager::RemovePass(const char* name) {
    m_Passes.erase(
        std::remove_if(m_Passes.begin(), m_Passes.end(),
            [name](const std::unique_ptr<RenderPass>& pass) {
                return std::string(pass->GetName()) == name;
            }),
        m_Passes.end()
    );
}

RenderPass* PassManager::GetPass(const char* name) {
    for (auto& pass : m_Passes) {
        if (std::string(pass->GetName()) == name) {
            return pass.get();
        }
    }
    return nullptr;
}

void PassManager::OnCreate(RenderBackend* backend) {
    for (auto& pass : m_Passes) {
        pass->OnCreate(backend);
    }
}

void PassManager::OnDestroy() {
    for (auto& pass : m_Passes) {
        pass->OnDestroy();
    }
    m_Passes.clear();
}

void PassManager::OnResize(uint32_t width, uint32_t height) {
    for (auto& pass : m_Passes) {
        pass->OnResize(width, height);
    }
}

void PassManager::Execute(const SceneView& view, Registry* registry) {
    for (auto& pass : m_Passes) {
        if (pass->IsEnabled()) {
            pass->Execute(view, registry);
        }
    }
}

void PassManager::SetPassEnabled(const char* name, bool enabled) {
    RenderPass* pass = GetPass(name);
    if (pass) {
        pass->SetEnabled(enabled);
    }
}

std::vector<RenderPass*> PassManager::GetAllPasses() {
    std::vector<RenderPass*> passes;
    for (auto& pass : m_Passes) {
        passes.push_back(pass.get());
    }
    return passes;
}

void PassManager::SortPassesByPriority() {
    std::sort(m_Passes.begin(), m_Passes.end(),
        [](const std::unique_ptr<RenderPass>& a, const std::unique_ptr<RenderPass>& b) {
            return a->GetPriority() < b->GetPriority();
        });
}

} // namespace MyEngine
