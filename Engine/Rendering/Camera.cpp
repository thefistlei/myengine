/******************************************************************************
 * File: Camera.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Camera implementation
 ******************************************************************************/

#include "Camera.h"

namespace MyEngine {

SceneCamera::SceneCamera() {
    RecalculateProjection();
}

void SceneCamera::SetOrthographic(float size, float nearClip, float farClip) {
    m_ProjectionType = ProjectionType::Orthographic;
    m_OrthographicSize = size;
    m_OrthographicNear = nearClip;
    m_OrthographicFar = farClip;
    RecalculateProjection();
}

void SceneCamera::SetPerspective(float degVerticalFOV, float nearClip, float farClip) {
    m_ProjectionType = ProjectionType::Perspective;
    m_PerspectiveFOV = degVerticalFOV;
    m_PerspectiveNear = nearClip;
    m_PerspectiveFar = farClip;
    RecalculateProjection();
}

void SceneCamera::SetViewportSize(uint32_t width, uint32_t height) {
    m_AspectRatio = (float)width / (float)height;
    RecalculateProjection();
}

void SceneCamera::RecalculateProjection() {
    if (m_ProjectionType == ProjectionType::Perspective) {
        float radians = m_PerspectiveFOV * (3.14159265f / 180.0f);
        m_Projection = Mat4::Perspective(radians, m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
    } else {
        float left = -m_OrthographicSize * m_AspectRatio * 0.5f;
        float right = m_OrthographicSize * m_AspectRatio * 0.5f;
        float bottom = -m_OrthographicSize * 0.5f;
        float top = m_OrthographicSize * 0.5f;

        m_Projection = Mat4::Ortho(left, right, bottom, top, m_OrthographicNear, m_OrthographicFar);
    }
}

} // namespace MyEngine
