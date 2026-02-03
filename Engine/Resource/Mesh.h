/******************************************************************************
 * File: Mesh.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Mesh resource class
 ******************************************************************************/

#pragma once

#include <vector>
#include <memory>
#include "Math/MathTypes.h"
#include "Rendering/Buffer.h"
#include "Rendering/VertexArray.h"

namespace MyEngine {

/**
 * @brief Vertex data structure
 */
struct Vertex {
    Vec3 Position;
    Vec3 Normal;
    Vec2 TexCoord;
};

/**
 * @brief Mesh resource representing a collection of vertices and indices
 */
class Mesh {
public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    ~Mesh() = default;
    
    const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
    const std::vector<uint32_t>& GetIndices() const { return m_Indices; }
    
    const std::shared_ptr<VertexArray>& GetVertexArray() const { return m_VertexArray; }
    
private:
    std::vector<Vertex> m_Vertices;
    std::vector<uint32_t> m_Indices;
    
    std::shared_ptr<VertexArray> m_VertexArray;
};

/**
 * @brief Static utility for loading meshes
 */
class MeshLoader {
public:
    static std::shared_ptr<Mesh> Load(const std::string& filepath);
};

} // namespace MyEngine
