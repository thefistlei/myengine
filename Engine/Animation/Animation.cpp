/******************************************************************************
 * File: Animation.cpp
 * Description: Animation loading with Assimp
 ******************************************************************************/

#include "Animation.h"
#include "AnimatedModelLoader.h"
#include "Core/Log.h"

#ifdef USE_ASSIMP
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace MyEngine {

// Helper to convert assimp matrix to our Mat4
static Mat4 AssimpToMat4(const aiMatrix4x4& from) {
    Mat4 to;
    to.m[0] = from.a1; to.m[4] = from.a2; to.m[8] = from.a3; to.m[12] = from.a4;
    to.m[1] = from.b1; to.m[5] = from.b2; to.m[9] = from.b3; to.m[13] = from.b4;
    to.m[2] = from.c1; to.m[6] = from.c2; to.m[10] = from.c3; to.m[14] = from.c4;
    to.m[3] = from.d1; to.m[7] = from.d2; to.m[11] = from.d3; to.m[15] = from.d4;
    return to;
}

// Helper to convert assimp vector to our Vec3
static Vec3 AssimpToVec3(const aiVector3D& vec) {
    return Vec3(vec.x, vec.y, vec.z);
}

// Helper to convert assimp quaternion to our Quat
static Quat AssimpToQuat(const aiQuaternion& q) {
    return Quat(q.x, q.y, q.z, q.w);
}

Animation::Animation(const std::string& animationPath, AnimatedModelLoader* model) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    
    if (!scene || !scene->mRootNode || !scene->HasAnimations()) {
        ENGINE_ERROR("Failed to load animation from {}", animationPath);
        return;
    }
    
    auto animation = scene->mAnimations[0];
    m_Duration = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;
    
    ReadHierarchyData(m_RootNode, scene->mRootNode);
    ReadMissingBones(animation, *model);
    
    ENGINE_INFO("Animation loaded: duration={}, tps={}, bones={}", 
                m_Duration, m_TicksPerSecond, m_Bones.size());
}

Bone* Animation::FindBone(const std::string& name) {
    auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
        [&](const Bone& bone) {
            return bone.GetBoneName() == name;
        }
    );
    
    if (iter == m_Bones.end())
        return nullptr;
    else
        return &(*iter);
}

void Animation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src) {
    dest.name = src->mName.data;
    dest.transformation = AssimpToMat4(src->mTransformation);
    dest.childrenCount = src->mNumChildren;

    for (int i = 0; i < src->mNumChildren; i++) {
        AssimpNodeData newData;
        ReadHierarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}

void Animation::ReadMissingBones(const aiAnimation* animation, AnimatedModelLoader& model) {
    int size = animation->mNumChannels;

    auto& boneInfoMap = model.GetBoneInfoMap();
    int& boneCount = model.GetBoneCount();

    // Reading channels (bones engaged in an animation and their keyframes)
    for (int i = 0; i < size; i++) {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
            boneInfoMap[boneName].id = boneCount;
            boneCount++;
        }
        
        m_BoneInfoMap = boneInfoMap;
        
        Bone newBone(boneName, boneInfoMap[boneName].id, boneInfoMap[boneName].offset);
        
        // Read position keys
        for (int positionIndex = 0; positionIndex < channel->mNumPositionKeys; ++positionIndex) {
            aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
            float timeStamp = channel->mPositionKeys[positionIndex].mTime;
            newBone.AddPositionKey(AssimpToVec3(aiPosition), timeStamp);
        }

        // Read rotation keys
        for (int rotationIndex = 0; rotationIndex < channel->mNumRotationKeys; ++rotationIndex) {
            aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
            float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
            newBone.AddRotationKey(AssimpToQuat(aiOrientation), timeStamp);
        }

        // Read scale keys
        for (int keyIndex = 0; keyIndex < channel->mNumScalingKeys; ++keyIndex) {
            aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
            float timeStamp = channel->mScalingKeys[keyIndex].mTime;
            newBone.AddScaleKey(AssimpToVec3(scale), timeStamp);
        }
        
        m_Bones.push_back(newBone);
    }
}

} // namespace MyEngine

#else

namespace MyEngine {

Animation::Animation(const std::string& animationPath, AnimatedModelLoader* model) {
    ENGINE_ERROR("Animation loading requires Assimp. Please enable USE_ASSIMP.");
}

Bone* Animation::FindBone(const std::string& name) {
    return nullptr;
}

void Animation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src) {}
void Animation::ReadMissingBones(const aiAnimation* animation, AnimatedModelLoader& model) {}

} // namespace MyEngine

#endif // USE_ASSIMP
