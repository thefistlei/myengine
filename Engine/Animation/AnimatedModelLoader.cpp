/******************************************************************************
 * File: AnimatedModelLoader.cpp
 * Description: Implementation of animated model loader with bone extraction
 ******************************************************************************/

#include "AnimatedModelLoader.h"
#include "Core/Log.h"
#include <cassert>

#ifdef USE_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
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

std::shared_ptr<AnimatedMesh> AnimatedModelLoader::LoadMesh(const std::string& filepath) {
    ENGINE_INFO("Loading animated mesh: {}", filepath);
    
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filepath,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        ENGINE_ERROR("Failed to load animated model: {}", importer.GetErrorString());
        return nullptr;
    }
    
    std::vector<AnimatedVertex> vertices;
    std::vector<uint32_t> indices;
    
    ProcessNode(scene->mRootNode, scene, vertices, indices);
    
    ENGINE_INFO("Loaded animated mesh: {} vertices, {} indices, {} bones", 
                vertices.size(), indices.size(), m_BoneCounter);
    
    return std::make_shared<AnimatedMesh>(vertices, indices);
}

std::shared_ptr<Animation> AnimatedModelLoader::LoadAnimation(const std::string& filepath) {
    ENGINE_INFO("Loading animation: {}", filepath);
    
    auto animation = std::make_shared<Animation>(filepath, this);
    
    return animation;
}

void AnimatedModelLoader::ProcessNode(aiNode* node, const aiScene* scene,
                                      std::vector<AnimatedVertex>& vertices,
                                      std::vector<uint32_t>& indices) {
    // Process all meshes in this node
    for (uint32_t i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh, scene, vertices, indices);
    }
    
    // Recursively process child nodes
    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene, vertices, indices);
    }
}

void AnimatedModelLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene,
                                      std::vector<AnimatedVertex>& vertices,
                                      std::vector<uint32_t>& indices) {
    uint32_t vertexOffset = static_cast<uint32_t>(vertices.size());
    
    // Process vertices
    for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
        AnimatedVertex vertex;
        
        // Position
        vertex.Position.x = mesh->mVertices[i].x;
        vertex.Position.y = mesh->mVertices[i].y;
        vertex.Position.z = mesh->mVertices[i].z;
        
        // Normal
        if (mesh->HasNormals()) {
            vertex.Normal.x = mesh->mNormals[i].x;
            vertex.Normal.y = mesh->mNormals[i].y;
            vertex.Normal.z = mesh->mNormals[i].z;
        }
        
        // Texture Coordinates
        if (mesh->mTextureCoords[0]) {
            vertex.TexCoord.x = mesh->mTextureCoords[0][i].x;
            vertex.TexCoord.y = mesh->mTextureCoords[0][i].y;
        }
        
        // Tangent
        if (mesh->HasTangentsAndBitangents()) {
            vertex.Tangent.x = mesh->mTangents[i].x;
            vertex.Tangent.y = mesh->mTangents[i].y;
            vertex.Tangent.z = mesh->mTangents[i].z;
            
            vertex.Bitangent.x = mesh->mBitangents[i].x;
            vertex.Bitangent.y = mesh->mBitangents[i].y;
            vertex.Bitangent.z = mesh->mBitangents[i].z;
        }
        
        vertices.push_back(vertex);
    }
    
    // Process indices
    for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; j++) {
            indices.push_back(vertexOffset + face.mIndices[j]);
        }
    }
    
    // Extract bone weights
    ExtractBoneWeightForVertices(vertices, mesh, scene);
}

void AnimatedModelLoader::ExtractBoneWeightForVertices(std::vector<AnimatedVertex>& vertices,
                                                       aiMesh* mesh, const aiScene* scene) {
    for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        
        if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()) {
            BoneInfo newBoneInfo;
            newBoneInfo.id = m_BoneCounter;
            newBoneInfo.offset = AssimpToMat4(mesh->mBones[boneIndex]->mOffsetMatrix);
            m_BoneInfoMap[boneName] = newBoneInfo;
            boneID = m_BoneCounter;
            m_BoneCounter++;
        } else {
            boneID = m_BoneInfoMap[boneName].id;
        }
        
        assert(boneID != -1);
        
        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;
        
        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            assert(vertexId < vertices.size());
            SetVertexBoneData(vertices[vertexId], boneID, weight);
        }
    }
}

void AnimatedModelLoader::SetVertexBoneData(AnimatedVertex& vertex, int boneID, float weight) {
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
        if (vertex.BoneIDs[i] < 0) {
            vertex.Weights[i] = weight;
            vertex.BoneIDs[i] = boneID;
            break;
        }
    }
}

} // namespace MyEngine

#else

namespace MyEngine {

std::shared_ptr<AnimatedMesh> AnimatedModelLoader::LoadMesh(const std::string& filepath) {
    ENGINE_ERROR("AnimatedModelLoader requires Assimp. Please enable USE_ASSIMP.");
    return nullptr;
}

std::shared_ptr<Animation> AnimatedModelLoader::LoadAnimation(const std::string& filepath) {
    ENGINE_ERROR("AnimatedModelLoader requires Assimp. Please enable USE_ASSIMP.");
    return nullptr;
}

void AnimatedModelLoader::ProcessNode(aiNode* node, const aiScene* scene,
                                      std::vector<AnimatedVertex>& vertices,
                                      std::vector<uint32_t>& indices) {}

void AnimatedModelLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene,
                                      std::vector<AnimatedVertex>& vertices,
                                      std::vector<uint32_t>& indices) {}

void AnimatedModelLoader::ExtractBoneWeightForVertices(std::vector<AnimatedVertex>& vertices,
                                                       aiMesh* mesh, const aiScene* scene) {}

void AnimatedModelLoader::SetVertexBoneData(AnimatedVertex& vertex, int boneID, float weight) {}

} // namespace MyEngine

#endif // USE_ASSIMP
