/******************************************************************************
 * File: AssimpMeshLoader.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Assimp-based mesh loader implementation
 ******************************************************************************/

#include "AssimpMeshLoader.h"
#include "Core/Log.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace MyEngine {

std::shared_ptr<Mesh> AssimpMeshLoader::Load(const std::string& filepath) {
    ENGINE_INFO("Loading mesh with Assimp: {}", filepath);
    
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filepath, 
        aiProcess_Triangulate | 
        aiProcess_FlipUVs | 
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        ENGINE_ERROR("Assimp failed to load model: {}", importer.GetErrorString());
        
        // Fallback: Return a simple quad
        std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f,  0.0f}, {0, 0, 1}, {0, 0}},
            {{ 0.5f, -0.5f,  0.0f}, {0, 0, 1}, {1, 0}},
            {{ 0.5f,  0.5f,  0.0f}, {0, 0, 1}, {1, 1}},
            {{-0.5f,  0.5f,  0.0f}, {0, 0, 1}, {0, 1}}
        };
        std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };
        return std::make_shared<Mesh>(vertices, indices);
    }
    
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    ProcessNode(scene->mRootNode, scene, vertices, indices);
    
    ENGINE_INFO("Loaded mesh: {} vertices, {} indices", vertices.size(), indices.size());
    
    return std::make_shared<Mesh>(vertices, indices);
}

void AssimpMeshLoader::ProcessNode(aiNode* node, const aiScene* scene, 
                                   std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
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

void AssimpMeshLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene,
                                   std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    uint32_t vertexOffset = static_cast<uint32_t>(vertices.size());
    
    // Process vertices
    for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        
        // Position
        vertex.Position.x = mesh->mVertices[i].x;
        vertex.Position.y = mesh->mVertices[i].y;
        vertex.Position.z = mesh->mVertices[i].z;
        
        // Normal
        if (mesh->HasNormals()) {
            vertex.Normal.x = mesh->mNormals[i].x;
            vertex.Normal.y = mesh->mNormals[i].y;
            vertex.Normal.z = mesh->mNormals[i].z;
        } else {
            vertex.Normal = Vec3(0, 1, 0);
        }
        
        // Texture Coordinates
        if (mesh->mTextureCoords[0]) {
            vertex.TexCoord.x = mesh->mTextureCoords[0][i].x;
            vertex.TexCoord.y = mesh->mTextureCoords[0][i].y;
        } else {
            vertex.TexCoord = Vec2(0, 0);
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
}

} // namespace MyEngine
