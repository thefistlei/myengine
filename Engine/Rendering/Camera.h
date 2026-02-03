/******************************************************************************
 * File: Camera.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Camera abstraction
 ******************************************************************************/

#pragma once

#include "Math/MathTypes.h"

namespace MyEngine {

/**
 * @brief Camera projection type
 */
enum class ProjectionType { Perspective = 0, Orthographic = 1 };

/**
 * @brief Base camera class
 */
class Camera {
public:
    Camera() = default;
    Camera(const Mat4& projection) : m_Projection(projection) {}
    virtual ~Camera() = default;

    const Mat4& GetProjection() const { return m_Projection; }

protected:
    Mat4 m_Projection;
};

/**
 * @brief Perspective and Orthographic camera for scenes
 */
class SceneCamera : public Camera {
public:
    SceneCamera();
    virtual ~SceneCamera() = default;

    void SetOrthographic(float size, float nearClip, float farClip);
    void SetPerspective(float degVerticalFOV, float nearClip, float farClip);

    void SetViewportSize(uint32_t width, uint32_t height);

    ProjectionType GetProjectionType() const { return m_ProjectionType; }
    void SetProjectionType(ProjectionType type) { m_ProjectionType = type; RecalculateProjection(); }

private:
    void RecalculateProjection();

private:
    ProjectionType m_ProjectionType = ProjectionType::Orthographic;

    float m_OrthographicSize = 10.0f;
    float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;

    float m_PerspectiveFOV = 45.0f;
    float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;

    float m_AspectRatio = 0.0f;
};

} // namespace MyEngine
