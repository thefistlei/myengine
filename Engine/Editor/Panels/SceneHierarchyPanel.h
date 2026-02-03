/******************************************************************************
 * File: SceneHierarchyPanel.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Scene hierarchy panel for entity tree view
 *              Based on 3D_Engine_Editor_UI_Architecture_Guide.md Section 10.2
 ******************************************************************************/

#pragma once

#include "Editor/Panel.h"
#include "ECS/Registry.h"
#include "ECS/Entity.h"
#include "ECS/Components.h"

namespace MyEngine {

/**
 * @brief 场景层级面板
 * 
 * 显示场景中所有实体的树形结构：
 * - 实体列表展示
 * - 实体选择
 * - 右键菜单（创建、删除实体）
 * - 支持父子层级关系显示
 */
class SceneHierarchyPanel : public Panel {
public:
    SceneHierarchyPanel(Registry* context = nullptr);
    virtual ~SceneHierarchyPanel() = default;
    
    virtual void OnUIRender() override;
    virtual void OnUpdate(float deltaTime) override;
    
    /**
     * @brief 设置场景上下文
     */
    void SetContext(Registry* context) { m_Context = context; }
    
    /**
     * @brief 获取选中的实体
     */
    Entity GetSelectedEntity() const { return m_SelectionContext; }
    
    /**
     * @brief 设置选中的实体
     */
    void SetSelectedEntity(Entity entity) { m_SelectionContext = entity; }
    
private:
    /**
     * @brief Draw entity node (recursive)
     */
    void DrawEntityNode(Entity entity);
    
    /**
     * @brief Draw context menu
     */
    void DrawContextMenu();
    
    /**
     * @brief Helper functions for entity operations
     */
    void DeleteEntity(Entity entity);
    Entity DuplicateEntity(Entity entity);
    Entity CreateChildEntity(Entity parent);
    
private:
    Registry* m_Context = nullptr;      // Scene context
    Entity m_SelectionContext;          // Currently selected entity
    
    // Rename dialog state
    bool m_RenamingEntity = false;
    Entity m_EntityToRename;
    char m_RenameBuffer[256] = "New Entity";
};

} // namespace MyEngine
