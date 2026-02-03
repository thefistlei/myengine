/******************************************************************************
 * File: AnimatedMesh.h
 * Description: Mesh with skeletal animation support
 ******************************************************************************/

#pragma once

#include "Resource/Mesh.h"
#include "Animation/Bone.h"
#include <map>

namespace MyEngine {

/**
 * @brief Vertex with skeletal animation data
 */
struct AnimatedVertex {
    Vec3 Position;
    Vec3 Normal;
    Vec2 TexCoord;
    Vec3 Tangent;
    Vec3 Bitangent;
    
    // Bone IDs that influence this vertex (up to 4 bones)
    int BoneIDs[MAX_BONE_INFLUENCE];
    
    // Weights for each bone influence
    float Weights[MAX_BONE_INFLUENCE];
    
    AnimatedVertex() {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            BoneIDs[i] = -1;
            Weights[i] = 0.0f;
        }
    }
};

/**
 * @brief Mesh with skeletal animation support
 */
class AnimatedMesh {
public:
    AnimatedMesh(const std::vector<AnimatedVertex>& vertices, const std::vector<uint32_t>& indices);
    ~AnimatedMesh() = default;
    
    const std::vector<AnimatedVertex>& GetVertices() const { return m_Vertices; }
    const std::vector<uint32_t>& GetIndices() const { return m_Indices; }
    
    const std::shared_ptr<VertexArray>& GetVertexArray() const { return m_VertexArray; }
    
private:
    std::vector<AnimatedVertex> m_Vertices;
    std::vector<uint32_t> m_Indices;
    
    std::shared_ptr<VertexArray> m_VertexArray;
};

} // namespace MyEngine
