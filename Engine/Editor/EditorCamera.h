/******************************************************************************
 * File: EditorCamera.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Editor camera with orbit/pan controls
 *              Based on 3D_Engine_Editor_UI_Architecture_Guide.md
 ******************************************************************************/

#pragma once

#include "Rendering/Camera.h"
#include "Math/MathTypes.h"
#include "Core/Event.h"
#include "Core/MouseEvent.h"

namespace MyEngine {

/**
 * @brief 编辑器摄像机
 * 
 * 提供编辑器中的摄像机控制功能：
 * - 鼠标中键拖拽：平移
 * - 鼠标右键拖拽：旋转（轨道）
 * - 滚轮：缩放
 */
class EditorCamera {
public:
    EditorCamera(float fov = 45.0f, float aspectRatio = 16.0f / 9.0f, 
                 float nearClip = 0.1f, float farClip = 1000.0f);
    
    /**
     * @brief 更新摄像机状态
     * @param deltaTime 帧时间
     */
    void OnUpdate(float deltaTime);
    
    /**
     * @brief 处理事件
     */
    void OnEvent(Event& event);
    
    /**
     * @brief 获取视图矩阵
     */
    Mat4 GetViewMatrix() const;
    
    /**
     * @brief 获取投影矩阵
     */
    Mat4 GetProjectionMatrix() const { return m_ProjectionMatrix; }
    
    /**
     * @brief 获取视图投影矩阵
     */
    Mat4 GetViewProjectionMatrix() const { return m_ProjectionMatrix * GetViewMatrix(); }
    
    /**
     * @brief 设置视口尺寸
     */
    void SetViewportSize(float width, float height);
    
    /**
     * @brief 获取/设置焦点点
     */
    const Vec3& GetFocalPoint() const { return m_FocalPoint; }
    void SetFocalPoint(const Vec3& point) { m_FocalPoint = point; }
    
    /**
     * @brief 获取/设置距离
     */
    float GetDistance() const { return m_Distance; }
    void SetDistance(float distance) { m_Distance = distance; }
    
    /**
     * @brief 获取/设置pitch和yaw角度
     */
    float GetPitch() const { return m_Pitch; }
    void SetPitch(float pitch) { m_Pitch = pitch; }
    float GetYaw() const { return m_Yaw; }
    void SetYaw(float yaw) { m_Yaw = yaw; }
    
    /**
     * @brief Get camera position
     */
    Vec3 GetPosition() const;
    
    /**
     * @brief Mouse input handlers for camera control
     */
    void MousePan(const Vec2& delta);
    void MouseRotate(const Vec2& delta);
    void MouseZoom(float delta);
    
    /**
     * @brief Keyboard movement
     */
    void KeyboardMove(float deltaTime);
    
    /**
     * @brief Update view matrix
     */
    void UpdateView();
    
    /**
     * @brief Camera settings
     */
    struct Settings {
        float movementSpeed = 8.0f;
        float panSpeed = 0.5f;
        float rotationSpeed = 0.2f;
        float zoomSpeed = 0.2f;
        
        float minDistance = 0.1f;
        float maxDistance = 1000.0f;
        
        float minPitch = -89.0f;
        float maxPitch = 89.0f;
        
        float shiftMultiplier = 2.0f;
        float ctrlMultiplier = 0.5f;
    };
    
    void SetSettings(const Settings& settings) { m_Settings = settings; }
    const Settings& GetSettings() const { return m_Settings; }
    
private:
    void UpdateProjection();
    
    bool OnMouseScroll(MouseScrolledEvent& e);
    
    Vec3 CalculatePosition() const;
    
private:
    float m_FOV = 45.0f;
    float m_AspectRatio = 16.0f / 9.0f;
    float m_NearClip = 0.1f;
    float m_FarClip = 1000.0f;
    
    Mat4 m_ProjectionMatrix;
    Mat4 m_ViewMatrix;
    
    Vec3 m_Position = Vec3(0, 25, 50);   // Higher and farther to view terrain
    Vec3 m_FocalPoint = Vec3(0, 0, 0);
    
    Vec2 m_InitialMousePosition;
    
    float m_Distance = 55.0f;            // Increased distance for better terrain view
    float m_Pitch = -20.0f;              // Slight downward angle
    float m_Yaw = 0.0f;                  // Y-axis rotation (left/right)
    
    float m_ViewportWidth = 1280.0f;
    float m_ViewportHeight = 720.0f;
    
    Settings m_Settings;
};

} // namespace MyEngine
