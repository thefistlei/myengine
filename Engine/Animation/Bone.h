/******************************************************************************
 * File: Bone.h
 * Description: Bone structure for skeletal animation
 ******************************************************************************/

#pragma once

#include "Math/MathTypes.h"
#include <string>
#include <vector>

namespace MyEngine {

// Maximum bone influence per vertex
#define MAX_BONE_INFLUENCE 4

struct KeyPosition {
    Vec3 position;
    float timeStamp;
};

struct KeyRotation {
    Quat orientation;
    float timeStamp;
};

struct KeyScale {
    Vec3 scale;
    float timeStamp;
};

class Bone {
public:
    Bone(const std::string& name, int id, const Mat4& offsetMatrix)
        : m_Name(name)
        , m_ID(id)
        , m_LocalTransform(Mat4())
        , m_OffsetMatrix(offsetMatrix)
    {}

    void Update(float animationTime);
    
    Mat4 GetLocalTransform() const { return m_LocalTransform; }
    const std::string& GetBoneName() const { return m_Name; }
    int GetBoneID() const { return m_ID; }
    const Mat4& GetOffsetMatrix() const { return m_OffsetMatrix; }

    // Add keyframes
    void AddPositionKey(const Vec3& position, float time) {
        m_Positions.push_back({ position, time });
    }
    
    void AddRotationKey(const Quat& rotation, float time) {
        m_Rotations.push_back({ rotation, time });
    }
    
    void AddScaleKey(const Vec3& scale, float time) {
        m_Scales.push_back({ scale, time });
    }

    int GetPositionIndex(float animationTime) const;
    int GetRotationIndex(float animationTime) const;
    int GetScaleIndex(float animationTime) const;

private:
    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
    
    Mat4 InterpolatePosition(float animationTime);
    Mat4 InterpolateRotation(float animationTime);
    Mat4 InterpolateScaling(float animationTime);

    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale> m_Scales;
    
    Mat4 m_LocalTransform;
    Mat4 m_OffsetMatrix;  // Transform from model space to bone space
    std::string m_Name;
    int m_ID;
};

} // namespace MyEngine
