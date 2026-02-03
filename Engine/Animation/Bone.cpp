/******************************************************************************
 * File: Bone.cpp
 * Description: Bone animation implementation with keyframe interpolation
 ******************************************************************************/

#include "Bone.h"
#include <algorithm>

namespace MyEngine {

void Bone::Update(float animationTime) {
    Mat4 translation = InterpolatePosition(animationTime);
    Mat4 rotation = InterpolateRotation(animationTime);
    Mat4 scale = InterpolateScaling(animationTime);
    m_LocalTransform = translation * rotation * scale;
}

int Bone::GetPositionIndex(float animationTime) const {
    for (int index = 0; index < m_Positions.size() - 1; ++index) {
        if (animationTime < m_Positions[index + 1].timeStamp)
            return index;
    }
    return 0;
}

int Bone::GetRotationIndex(float animationTime) const {
    for (int index = 0; index < m_Rotations.size() - 1; ++index) {
        if (animationTime < m_Rotations[index + 1].timeStamp)
            return index;
    }
    return 0;
}

int Bone::GetScaleIndex(float animationTime) const {
    for (int index = 0; index < m_Scales.size() - 1; ++index) {
        if (animationTime < m_Scales[index + 1].timeStamp)
            return index;
    }
    return 0;
}

float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

Mat4 Bone::InterpolatePosition(float animationTime) {
    if (m_Positions.size() == 1)
        return Mat4::Translation(m_Positions[0].position);

    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
                                       m_Positions[p1Index].timeStamp, animationTime);
    
    Vec3 finalPosition = Vec3::Lerp(m_Positions[p0Index].position,
                                     m_Positions[p1Index].position, scaleFactor);
    return Mat4::Translation(finalPosition);
}

Mat4 Bone::InterpolateRotation(float animationTime) {
    if (m_Rotations.size() == 1) {
        Quat rotation = m_Rotations[0].orientation.Normalized();
        return rotation.ToMatrix();
    }

    int p0Index = GetRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp,
                                       m_Rotations[p1Index].timeStamp, animationTime);
    
    Quat finalRotation = Quat::Slerp(m_Rotations[p0Index].orientation,
                                      m_Rotations[p1Index].orientation, scaleFactor);
    finalRotation = finalRotation.Normalized();
    return finalRotation.ToMatrix();
}

Mat4 Bone::InterpolateScaling(float animationTime) {
    if (m_Scales.size() == 1)
        return Mat4::Scale(m_Scales[0].scale);

    int p0Index = GetScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,
                                       m_Scales[p1Index].timeStamp, animationTime);
    
    Vec3 finalScale = Vec3::Lerp(m_Scales[p0Index].scale, m_Scales[p1Index].scale, scaleFactor);
    return Mat4::Scale(finalScale);
}

} // namespace MyEngine
