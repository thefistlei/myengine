/******************************************************************************
 * File: PropertiesPanel.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Properties panel for component editing
 *              Based on 3D_Engine_Editor_UI_Architecture_Guide.md Section 10.3
 ******************************************************************************/

#pragma once

#include "Editor/Panel.h"
#include "ECS/Entity.h"
#include "ECS/Components.h"

namespace MyEngine {

/**
 * @brief 属性面板
 * 
 * 显示和编辑选中实体的组件：
 * - 显示实体的所有组件
 * - 编辑组件属性
 * - 添加/删除组件
 * - 支持多种数据类型（Vector3、颜色、字符串等）
 */
class PropertiesPanel : public Panel {
public:
    PropertiesPanel();
    virtual ~PropertiesPanel() = default;
    
    virtual void OnUIRender() override;
    virtual void OnUpdate(float deltaTime) override;
    
    /**
     * @brief 设置要显示的实体
     */
    void SetSelectedEntity(Entity entity) { m_SelectionContext = entity; }
    
    /**
     * @brief 获取当前选中的实体
     */
    Entity GetSelectedEntity() const { return m_SelectionContext; }
    
private:
    /**
     * @brief 绘制所有组件
     */
    void DrawComponents(Entity entity);
    
    /**
     * @brief 绘制 TagComponent
     */
    void DrawTagComponent(Entity entity);
    
    /**
     * @brief 绘制 TransformComponent
     */
    void DrawTransformComponent(Entity entity);
    
    /**
     * @brief 绘制 HierarchyComponent
     */
    void DrawHierarchyComponent(Entity entity);
    
    /**
     * @brief 绘制 MeshFilterComponent
     */
    void DrawMeshFilterComponent(Entity entity);
    
    /**
     * @brief 绘制 PassComponent
     */
    void DrawPassComponent(Entity entity);
    
    /**
     * @brief 绘制添加组件按钮
     */
    void DrawAddComponentButton(Entity entity);
    
private:
    Entity m_SelectionContext;      // 当前选中的实体
};

} // namespace MyEngine
