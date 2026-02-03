/******************************************************************************
 * File: AssimpMeshLoader.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Assimp-based mesh loader supporting FBX, GLTF, OBJ, etc.
 ******************************************************************************/

#pragma once

#include "Mesh.h"
#include <memory>
#include <string>

// Forward declarations for Assimp types
struct aiNode;
struct aiScene;
struct aiMesh;

namespace MyEngine {

/**
 * @brief Assimp mesh loader supporting multiple 3D formats
 */
class AssimpMeshLoader {
public:
    /**
     * @brief Load mesh from file using Assimp
     * @param filepath Path to the model file (FBX, OBJ, GLTF, etc.)
     * @return Shared pointer to loaded mesh
     */
    static std::shared_ptr<Mesh> Load(const std::string& filepath);

private:
    static void ProcessNode(aiNode* node, const aiScene* scene, 
                           std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    static void ProcessMesh(aiMesh* mesh, const aiScene* scene,
                           std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
};

} // namespace MyEngine
