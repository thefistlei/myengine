/******************************************************************************
 * File: ViewportPanel.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Viewport panel implementation
 ******************************************************************************/

#include "ViewportPanel.h"
#include "Core/Log.h"
#include <imgui.h>
#include <glad/gl.h>

namespace MyEngine {

ViewportPanel::ViewportPanel(EditorCamera* camera)
    : Panel("Viewport", true), m_EditorCamera(camera)
{
}

ViewportPanel::~ViewportPanel() {
    // Cleanup framebuffer
    if (m_FramebufferID != 0) {
        glDeleteFramebuffers(1, &m_FramebufferID);
        glDeleteTextures(1, &m_ColorAttachmentID);
        glDeleteRenderbuffers(1, &m_DepthAttachmentID);
    }
}

void ViewportPanel::OnUIRender() {
    if (!m_IsOpen) return;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin(m_Name.c_str(), &m_IsOpen);
    ImGui::PopStyleVar();
    
    // Check viewport status
    m_ViewportFocused = ImGui::IsWindowFocused();
    m_ViewportHovered = ImGui::IsWindowHovered();
    
    // Get available area size
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    
    // If viewport size changed, adjust framebuffer size
    if (m_ViewportSize.x != viewportPanelSize.x || m_ViewportSize.y != viewportPanelSize.y) {
        m_ViewportSize = Vec2(viewportPanelSize.x, viewportPanelSize.y);
        
        // Notify editor camera
        if (m_EditorCamera) {
            m_EditorCamera->SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        }
        
        // Recreate framebuffer
        CreateFramebuffer((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        
        ENGINE_INFO("Viewport resized: {}x{}", (int)m_ViewportSize.x, (int)m_ViewportSize.y);
    }
    
    // Display framebuffer texture
    if (m_ColorAttachmentID != 0) {
        ImGui::Image(
            (ImTextureID)(intptr_t)m_ColorAttachmentID,
            ImVec2(m_ViewportSize.x, m_ViewportSize.y),
            ImVec2(0, 1), ImVec2(1, 0)  // Flip Y axis
        );
        
        // Drag-and-drop target
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
                // Get the asset GUID from payload
                const char* assetGUID = (const char*)payload->Data;
                
                // Calculate drop position in viewport coordinates
                ImVec2 mousePos = ImGui::GetMousePos();
                Vec2 dropPosition(
                    mousePos.x - m_ViewportBounds[0].x,
                    mousePos.y - m_ViewportBounds[0].y
                );
                
                // Invoke callback if set
                if (m_AssetDropCallback) {
                    m_AssetDropCallback(std::string(assetGUID), dropPosition);
                }
                
                ENGINE_INFO("Asset dropped in viewport: {} at ({}, {})", 
                    assetGUID, dropPosition.x, dropPosition.y);
            }
            ImGui::EndDragDropTarget();
        }
    } else {
        // Display placeholder when no texture
        ImGui::Text("Scene Viewport (Waiting for rendering)");
    }
    
    // Get viewport bounds (for mouse picking)
    auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
    auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
    auto viewportOffset = ImGui::GetWindowPos();
    
    m_ViewportBounds[0] = Vec2(
        viewportMinRegion.x + viewportOffset.x,
        viewportMinRegion.y + viewportOffset.y
    );
    m_ViewportBounds[1] = Vec2(
        viewportMaxRegion.x + viewportOffset.x,
        viewportMaxRegion.y + viewportOffset.y
    );
    
    ImGui::End();
}

void ViewportPanel::OnUpdate(float deltaTime) {
    // Can add viewport-specific update logic here
}

void ViewportPanel::OnEvent(Event& event) {
    // If viewport is focused, pass events to editor camera
    if (m_ViewportFocused || m_ViewportHovered) {
        if (m_EditorCamera) {
            m_EditorCamera->OnEvent(event);
        }
    }
}

void ViewportPanel::CreateFramebuffer(uint32_t width, uint32_t height) {
    // Prevent creating tiny framebuffers
    if (width == 0 || height == 0) {
        ENGINE_WARN("Invalid framebuffer size: {}x{}", width, height);
        return;
    }
    
    // Delete old framebuffer
    if (m_FramebufferID != 0) {
        glDeleteFramebuffers(1, &m_FramebufferID);
        glDeleteTextures(1, &m_ColorAttachmentID);
        glDeleteRenderbuffers(1, &m_DepthAttachmentID);
    }
    
    // Create framebuffer
    glGenFramebuffers(1, &m_FramebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
    
    // Create color texture attachment
    glGenTextures(1, &m_ColorAttachmentID);
    glBindTexture(GL_TEXTURE_2D, m_ColorAttachmentID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachmentID, 0);
    
    // Create depth/stencil renderbuffer attachment
    glGenRenderbuffers(1, &m_DepthAttachmentID);
    glBindRenderbuffer(GL_RENDERBUFFER, m_DepthAttachmentID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthAttachmentID);
    
    // Check framebuffer completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        ENGINE_ERROR("Framebuffer is not complete! Status: {}", status);
    } else {
        ENGINE_INFO("Created framebuffer: {}x{}, FBO={}, Color={}, Depth={}", 
            width, height, m_FramebufferID, m_ColorAttachmentID, m_DepthAttachmentID);
    }
    
    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace MyEngine
