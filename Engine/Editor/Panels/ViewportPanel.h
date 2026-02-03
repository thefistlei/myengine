/******************************************************************************
 * File: ViewportPanel.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Viewport panel for scene rendering
 *              Based on 3D_Engine_Editor_UI_Architecture_Guide.md Section 10.4
 ******************************************************************************/

#pragma once

#include "Editor/Panel.h"
#include "Editor/EditorCamera.h"
#include "Math/MathTypes.h"
#include <functional>

namespace MyEngine {

class Framebuffer;

/**
 * @brief 视口面板
 * 
 * 显示 3D 场景的渲染视口：
 * - 将场景渲染到帧缓冲区
 * - 在 ImGui 窗口中显示纹理
 * - 处理视口尺寸调整
 * - 支持编辑器摄像机控制
 * - 显示 Gizmo 工具
 */
class ViewportPanel : public Panel {
public:
    ViewportPanel(EditorCamera* camera = nullptr);
    virtual ~ViewportPanel();
    
    virtual void OnUIRender() override;
    virtual void OnUpdate(float deltaTime) override;
    virtual void OnEvent(Event& event) override;
    
    /**
     * @brief 设置编辑器摄像机
     */
    void SetEditorCamera(EditorCamera* camera) { m_EditorCamera = camera; }
    
    /**
     * @brief 获取视口尺寸
     */
    Vec2 GetViewportSize() const { return m_ViewportSize; }
    
    /**
     * @brief 视口是否被聚焦
     */
    bool IsFocused() const { return m_ViewportFocused; }
    
    /**
     * @brief 视口是否被悬停
     */
    bool IsHovered() const { return m_ViewportHovered; }
    
    /**
     * @brief Get framebuffer ID
     */
    uint32_t GetFramebufferID() const { return m_FramebufferID; }
    
    /**
     * @brief Get viewport bounds (for overlay positioning)
     */
    const Vec2* GetViewportBounds() const { return m_ViewportBounds; }
    
    /**
     * @brief Set callback for asset drop events
     */
    using AssetDropCallback = std::function<void(const std::string& assetGUID, const Vec2& dropPosition)>;
    void SetAssetDropCallback(AssetDropCallback callback) { m_AssetDropCallback = callback; }
    
private:
    /**
     * @brief 创建帧缓冲区
     */
    void CreateFramebuffer(uint32_t width, uint32_t height);
    
private:
    EditorCamera* m_EditorCamera = nullptr;
    
    // 帧缓冲区（未来实现）
    // std::shared_ptr<Framebuffer> m_Framebuffer;
    uint32_t m_FramebufferID = 0;       // OpenGL 帧缓冲 ID
    uint32_t m_ColorAttachmentID = 0;   // 颜色纹理 ID
    uint32_t m_DepthAttachmentID = 0;   // 深度缓冲 ID
    
    Vec2 m_ViewportSize = Vec2(1280, 720);
    Vec2 m_ViewportBounds[2];           // 视口边界（用于鼠标拾取）
    
    bool m_ViewportFocused = false;
    bool m_ViewportHovered = false;
    
    // Drag-and-drop callback
    AssetDropCallback m_AssetDropCallback;
};

} // namespace MyEngine
