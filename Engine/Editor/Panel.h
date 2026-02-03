/******************************************************************************
 * File: Panel.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Base panel class for editor UI
 *              Based on 3D_Engine_Editor_UI_Architecture_Guide.md Section 3.3
 ******************************************************************************/

#pragma once

#include <string>

namespace MyEngine {

class Event;  // 前向声明

/**
 * @brief 编辑器面板基类
 * 
 * 所有编辑器面板的抽象基类，提供统一的生命周期管理和事件处理接口。
 * 
 * 使用方式：
 * - 继承此类实现具体面板（如场景层级、属性检视器等）
 * - 在 OnUIRender() 中使用 ImGui 绘制 UI
 * - 通过 m_IsOpen 控制面板的显示/隐藏
 */
class Panel {
public:
    Panel(const std::string& name, bool openByDefault = true)
        : m_Name(name), m_IsOpen(openByDefault) {}
    
    virtual ~Panel() = default;
    
    /**
     * @brief 渲染面板 UI（必须实现）
     * 使用 ImGui API 在此方法中绘制面板内容
     */
    virtual void OnUIRender() = 0;
    
    /**
     * @brief 每帧更新（可选）
     * @param deltaTime 距离上一帧的时间（秒）
     */
    virtual void OnUpdate(float deltaTime) {}
    
    /**
     * @brief 处理事件（可选）
     * @param event 要处理的事件
     */
    virtual void OnEvent(Event& event) {}
    
    // 访问器
    const std::string& GetName() const { return m_Name; }
    bool IsOpen() const { return m_IsOpen; }
    void SetOpen(bool open) { m_IsOpen = open; }
    void ToggleOpen() { m_IsOpen = !m_IsOpen; }
    
protected:
    std::string m_Name;      // 面板名称
    bool m_IsOpen;           // 面板是否打开
};

} // namespace MyEngine
