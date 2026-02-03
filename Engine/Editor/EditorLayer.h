/******************************************************************************
 * File: EditorLayer.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Main editor layer managing all panels
 *              Based on 3D_Engine_Editor_UI_Architecture_Guide.md Section 3.2
 ******************************************************************************/

#pragma once

#include "Core/Layer.h"
#include "Panel.h"
#include "EditorCamera.h"
#include "ECS/Registry.h"
#include "Scene/SceneNode.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/PropertiesPanel.h"
#include "Panels/ViewportPanel.h"
#include "Panels/AssetBrowserPanel.h"
#include "Rendering/Pass/RenderPass.h"
#include <vector>
#include <memory>

namespace MyEngine {

class Scene;
class Shader;
class ScriptSystem;

/**
 * @brief 编辑器主层
 * 
 * 负责管理所有编辑器面板和编辑器状态：
 * - 面板管理（场景层级、属性、视口等）
 * - 编辑器摄像机
 * - 选中的实体
 * - 场景管理
 */
class EditorLayer : public Layer {
public:
    EditorLayer();
    virtual ~EditorLayer();
    
    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate(float deltaTime) override;
    virtual void OnUIRender() override;
    virtual void OnEvent(Event& event) override;
    
    /**
     * @brief 设置活动场景
     */
    void SetActiveScene(Registry* registry);
    
    /**
     * @brief 获取选中的实体
     */
    EntityID GetSelectedEntity() const { return m_SelectedEntity; }
    
    /**
     * @brief 设置选中的实体
     */
    void SetSelectedEntity(EntityID entity) { m_SelectedEntity = entity; }
    
    /**
     * @brief Handle asset drop from Asset Browser to Viewport
     */
    void OnAssetDropped(const std::string& assetGUID, const Vec2& dropPosition);
    
    /**
     * @brief Convert screen position to world position using raycasting
     * @param screenPos Screen position relative to viewport (top-left origin)
     * @param viewportSize Size of the viewport
     * @return World position at a fixed distance from camera
     */
    Vec3 ScreenToWorld(const Vec2& screenPos, const Vec2& viewportSize) const;
    
    /**
     * @brief Enter play mode (switch to game/player mode)
     */
    void EnterPlayMode();
    
    /**
     * @brief Exit play mode (switch back to editor mode)
     */
    void ExitPlayMode();
    
    /**
     * @brief Check if currently in play mode
     */
    bool IsPlayMode() const { return m_IsPlaying; }
    
    /**
     * @brief Set the script system (for updating scripts)
     */
    void SetScriptSystem(ScriptSystem* scriptSystem) { m_ScriptSystem = scriptSystem; }
    
private:
    void InitializePanels();
    void UpdatePanels(float deltaTime);
    void RenderMenuBar();
    void RenderToolbar();
    void RenderViewport(ViewportPanel* viewport);
    void CreatePassEntity(const std::string& name, RenderPass* pass, const std::string& passType);
    
private:
    // 面板列表
    std::vector<std::shared_ptr<Panel>> m_Panels;
    
    // 场景和 ECS
    Registry* m_ActiveRegistry = nullptr;
    SceneGraph m_SceneGraph;
    
    // 编辑器状态
    EntityID m_SelectedEntity = 0;
    EditorCamera m_EditorCamera;
    
    // Editor settings
    bool m_ShowGrid = true;
    bool m_ShowGizmos = true;
    bool m_ShowDemoWindow = false;  // ImGui Demo window
    
    // Mouse input state for camera control
    bool m_ViewportHovered = false;
    bool m_ViewportFocused = false;
    Vec2 m_LastMousePosition{0.0f, 0.0f};
    
    // 渲染资源
    std::unique_ptr<Shader> m_ViewportShader;
    
    // Pass system
    PassManager m_PassManager;
    bool m_UsePassSystem = true;  // Toggle between old/new rendering
    bool m_UseSceneCamera = false;  // Toggle between EditorCamera and scene MainCamera
    
    // Play mode state
    bool m_IsPlaying = false;
    
    // Script system pointer (non-owning)
    ScriptSystem* m_ScriptSystem = nullptr;
};

} // namespace MyEngine
