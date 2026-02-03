/******************************************************************************
 * File: EditorCamera.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Editor camera implementation
 ******************************************************************************/

#include "EditorCamera.h"
#include "Core/Log.h"
#include <cmath>
#include <algorithm>
#include <imgui.h>

namespace MyEngine {

// 新增：角度->弧度转换常量
static constexpr float DEG2RAD = 3.14159265358979323846f / 180.0f;

EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
    : m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip)
{
    UpdateProjection();
    UpdateView();
}

void EditorCamera::OnUpdate(float deltaTime) {
    // 未来可以在这里添加平滑移动等功能
    UpdateView();
}

void EditorCamera::OnEvent(Event& event) {
    // 目前简化实现，实际应该通过事件调度器处理
    // EventDispatcher dispatcher(event);
    // dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(EditorCamera::OnMouseScroll));
}

Mat4 EditorCamera::GetViewMatrix() const {
    return m_ViewMatrix;
}

void EditorCamera::SetViewportSize(float width, float height) {
    m_ViewportWidth = width;
    m_ViewportHeight = height;
    m_AspectRatio = width / height;
    UpdateProjection();
}

Vec3 EditorCamera::GetPosition() const {
    return CalculatePosition();
}

void EditorCamera::UpdateProjection() {
    m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
    m_ProjectionMatrix = Mat4::Perspective(m_FOV, m_AspectRatio, m_NearClip, m_FarClip);
}

void EditorCamera::UpdateView() {
    m_Position = CalculatePosition();

    // Build view matrix (look at focal point)
    Vec3 forward = (m_FocalPoint - m_Position).Normalized();
    Vec3 right = Vec3::Cross(Vec3(0, 1, 0), forward).Normalized();
    Vec3 up = Vec3::Cross(forward, right);

    // Construct view matrix manually (LookAt equivalent)
    // View matrix = rotation * translation
    m_ViewMatrix.m[0] = right.x;
    m_ViewMatrix.m[1] = up.x;
    m_ViewMatrix.m[2] = -forward.x;
    m_ViewMatrix.m[3] = 0.0f;

    m_ViewMatrix.m[4] = right.y;
    m_ViewMatrix.m[5] = up.y;
    m_ViewMatrix.m[6] = -forward.y;
    m_ViewMatrix.m[7] = 0.0f;

    m_ViewMatrix.m[8] = right.z;
    m_ViewMatrix.m[9] = up.z;
    m_ViewMatrix.m[10] = -forward.z;
    m_ViewMatrix.m[11] = 0.0f;

    m_ViewMatrix.m[12] = -Vec3::Dot(right, m_Position);
    m_ViewMatrix.m[13] = -Vec3::Dot(up, m_Position);
    m_ViewMatrix.m[14] = Vec3::Dot(forward, m_Position);
    m_ViewMatrix.m[15] = 1.0f;
}

bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e) {
    // Unity 风格：直接把滚轮偏移传入缩放处理（不再乘 0.1f 的固定系数）
    float delta = e.GetYOffset();
    MouseZoom(delta);
    UpdateView();
    return false;
}

void EditorCamera::MousePan(const Vec2& delta) {
    m_FocalPoint.x -= delta.x * m_Settings.panSpeed * m_Distance;
    m_FocalPoint.y += delta.y * m_Settings.panSpeed * m_Distance;
}

void EditorCamera::MouseRotate(const Vec2& delta) {
    // Unity 风格右键：自由视角（free look）
    // 旋转时保持当前 m_Position 不变，通过调整 yaw/pitch 并更新 m_FocalPoint，
    // 使得随后 UpdateView() 重新计算位置时位置保持一致（行为类似 Unity 右键拖拽）。
    m_Yaw += delta.x * m_Settings.rotationSpeed;
    m_Pitch += delta.y * m_Settings.rotationSpeed;

    // 限制俯仰角（仍然以度为单位）
    m_Pitch = std::clamp(m_Pitch, m_Settings.minPitch, m_Settings.maxPitch);

    // 计算旋转后基于 yaw/pitch 和当前 distance 的偏移（与 CalculatePosition 保持一致的数学形式）
    float pitchRad = m_Pitch * DEG2RAD;
    float yawRad = m_Yaw * DEG2RAD;

    Vec3 offset;
    offset.x = m_Distance * std::cos(pitchRad) * std::sin(yawRad);
    offset.y = m_Distance * std::sin(pitchRad);
    offset.z = m_Distance * std::cos(pitchRad) * std::cos(yawRad);

    // 为了保持 m_Position 不变，更新焦点为： focal = position - offset
    m_FocalPoint = m_Position - offset;

    // 更新视图矩阵（UpdateView 会再次基于 focal/yaw/pitch/distance 计算 position）
    UpdateView();
}

void EditorCamera::MouseZoom(float delta) {
    // Unity 风格缩放（dolly）：按比例改变 distance（以当前 distance 为基准，使缩放随距离有感知）
    // delta 为滚轮偏移（通常为正负整数），zoomSpeed 是调节系数
    // 使用乘法比例，使缩放在较远时步进更大，较近时更精细
    float zoomFactor = 1.0f - delta * m_Settings.zoomSpeed * 0.1f; // 0.1f 用于调节灵敏度
    if (zoomFactor < 0.01f) zoomFactor = 0.01f; // 防止负值或过小
    m_Distance *= zoomFactor;

    // 限制距离范围
    m_Distance = std::clamp(m_Distance, m_Settings.minDistance, m_Settings.maxDistance);
}

void EditorCamera::KeyboardMove(float deltaTime) {
    ImGuiIO& io = ImGui::GetIO();

    // Calculate movement speed with modifiers
    float velocity = m_Settings.movementSpeed * deltaTime;

    // Shift = speed up, Ctrl = slow down
    if (io.KeyShift) {
        velocity *= m_Settings.shiftMultiplier;
    }
    if (io.KeyCtrl) {
        velocity *= m_Settings.ctrlMultiplier;
    }

    // Calculate direction vectors based on current view
    Vec3 forward = (m_FocalPoint - m_Position).Normalized();
    Vec3 right = Vec3::Cross(Vec3(0, 1, 0), forward).Normalized();
    Vec3 up = Vec3(0, 1, 0);

    // Track total movement
    Vec3 movement(0, 0, 0);

    // WASD movement
    if (ImGui::IsKeyDown(ImGuiKey_W)) {
        movement = movement + forward * velocity;
    }
    if (ImGui::IsKeyDown(ImGuiKey_S)) {
        movement = movement - forward * velocity;
    }
    if (ImGui::IsKeyDown(ImGuiKey_A)) {
        movement = movement + right * velocity;
    }
    if (ImGui::IsKeyDown(ImGuiKey_D)) {
        movement = movement - right * velocity;
    }

    // QE up/down movement
    if (ImGui::IsKeyDown(ImGuiKey_E)) {
        movement = movement + up * velocity;
    }
    if (ImGui::IsKeyDown(ImGuiKey_Q)) {
        movement = movement - up * velocity;
    }

    // Apply movement to focal point only
    // Position will be recalculated in UpdateView() based on yaw/pitch/distance
    m_FocalPoint = m_FocalPoint + movement;
}

Vec3 EditorCamera::CalculatePosition() const {
    // 使用度->弧度转换，修正之前直接用度作为弧度的错误
    float pitchRad = m_Pitch * DEG2RAD;
    float yawRad = m_Yaw * DEG2RAD;

    float x = m_Distance * std::cos(pitchRad) * std::sin(yawRad);
    float y = m_Distance * std::sin(pitchRad);
    float z = m_Distance * std::cos(pitchRad) * std::cos(yawRad);

    return m_FocalPoint + Vec3(x, y, z);
}

} // namespace MyEngine
