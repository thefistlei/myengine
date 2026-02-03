/******************************************************************************
 * File: AnimatedModelLoader.h
 * Description: Loader for animated models with skeletal animation support
 ******************************************************************************/

#pragma once

#include "AnimatedMesh.h"
#include "Animation.h"
#include <memory>
#include <string>
#include <map>

// Forward declarations for Assimp types
struct aiNode;
struct aiScene;
struct aiMesh;
struct aiAnimation;

namespace MyEngine {

/**
 * @brief Loader for animated models with skeletal animation
 */
class AnimatedModelLoader {
public:
    AnimatedModelLoader() : m_BoneCounter(0) {}
    
    /**
     * @brief Load animated mesh from file
     * @param filepath Path to the model file (FBX, DAE, etc.)
     * @return Shared pointer to loaded animated mesh
     */
    std::shared_ptr<AnimatedMesh> LoadMesh(const std::string& filepath);
    
    /**
     * @brief Load animation from file
     * @param filepath Path to the animation file
     * @return Shared pointer to loaded animation
     */
    std::shared_ptr<Animation> LoadAnimation(const std::string& filepath);
    
    // Getters for bone info (needed by Animation class)
    std::map<std::string, BoneInfo>& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }

private:
    void ProcessNode(aiNode* node, const aiScene* scene, 
                     std::vector<AnimatedVertex>& vertices, 
                     std::vector<uint32_t>& indices);
    
    void ProcessMesh(aiMesh* mesh, const aiScene* scene,
                     std::vector<AnimatedVertex>& vertices, 
                     std::vector<uint32_t>& indices);
    
    void ExtractBoneWeightForVertices(std::vector<AnimatedVertex>& vertices, 
                                      aiMesh* mesh, const aiScene* scene);
    
    void SetVertexBoneData(AnimatedVertex& vertex, int boneID, float weight);
    
    std::map<std::string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter;
};

} // namespace MyEngine
