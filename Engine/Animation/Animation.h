/******************************************************************************
 * File: Animation.h
 * Description: Animation container with bone hierarchy
 ******************************************************************************/

#pragma once

#include "Bone.h"
#include <map>
#include <string>
#include <vector>

struct aiAnimation;
struct aiNode;

namespace MyEngine {

#define MAX_BONES 100

struct BoneInfo {
    int id;
    Mat4 offset;
};

struct AssimpNodeData {
    Mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation {
public:
    Animation() = default;
    Animation(const std::string& animationPath, class AnimatedModelLoader* model);
    ~Animation() = default;

    Bone* FindBone(const std::string& name);
    
    float GetTicksPerSecond() const { return m_TicksPerSecond; }
    float GetDuration() const { return m_Duration; }
    const AssimpNodeData& GetRootNode() const { return m_RootNode; }
    const std::map<std::string, BoneInfo>& GetBoneIDMap() const { return m_BoneInfoMap; }

private:
    void ReadMissingBones(const aiAnimation* animation, AnimatedModelLoader& model);
    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);

    float m_Duration;
    int m_TicksPerSecond;
    std::vector<Bone> m_Bones;
    AssimpNodeData m_RootNode;
    std::map<std::string, BoneInfo> m_BoneInfoMap;
};

} // namespace MyEngine
