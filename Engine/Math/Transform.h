/******************************************************************************
 * File: Transform.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Transform component (Position/Rotation/Scale)
 ******************************************************************************/

#pragma once

#include "MathTypes.h"

namespace MyEngine {

/**
 * @brief Transform component for entities
 */
class Transform {
public:
    Transform()
        : m_Position(0, 0, 0)
        , m_Rotation(Quat::Identity())
        , m_Scale(1, 1, 1)
    {}

    // Position
    const Vec3& GetPosition() const { return m_Position; }
    void SetPosition(const Vec3& position) { m_Position = position; }

    // Rotation
    const Quat& GetRotation() const { return m_Rotation; }
    void SetRotation(const Quat& rotation) { m_Rotation = rotation; }

    // Scale
    const Vec3& GetScale() const { return m_Scale; }
    void SetScale(const Vec3& scale) { m_Scale = scale; }

    // Get transformation matrix
    Mat4 GetMatrix() const {
        Mat4 translation = Mat4::Translation(m_Position);
        Mat4 scale = Mat4::Scale(m_Scale);
        
        // For now, return simple T * S
        // In production, include rotation
        return translation;  // Simplified
    }

private:
    Vec3 m_Position;
    Quat m_Rotation;
    Vec3 m_Scale;
};

} // namespace MyEngine
