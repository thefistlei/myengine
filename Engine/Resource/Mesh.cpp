/******************************************************************************
 * File: Mesh.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Mesh resource implementation
 ******************************************************************************/

#include "Mesh.h"
#include "Core/Log.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace MyEngine {

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    : m_Vertices(vertices), m_Indices(indices) {
    
    m_VertexArray.reset(VertexArray::Create());
    
    auto vb = std::shared_ptr<VertexBuffer>(VertexBuffer::Create((float*)vertices.data(), (uint32_t)(vertices.size() * sizeof(Vertex))));
    vb->SetLayout({
        { ShaderDataType::Float3, "a_Position" },
        { ShaderDataType::Float3, "a_Normal" },
        { ShaderDataType::Float2, "a_TexCoord" }
    });
    
    auto ib = std::shared_ptr<IndexBuffer>(IndexBuffer::Create((uint32_t*)indices.data(), (uint32_t)indices.size()));
    
    m_VertexArray->AddVertexBuffer(vb);
    m_VertexArray->SetIndexBuffer(ib);
}

std::shared_ptr<Mesh> MeshLoader::Load(const std::string& filepath) {
    ENGINE_INFO("Loading mesh: {}", filepath);
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        ENGINE_ERROR("Failed to open mesh file: {}", filepath);
        // Fallback to simple quad
        std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f,  0.0f}, {0, 0, 1}, {0, 0}},
            {{ 0.5f, -0.5f,  0.0f}, {0, 0, 1}, {1, 0}},
            {{ 0.5f,  0.5f,  0.0f}, {0, 0, 1}, {1, 1}},
            {{-0.5f,  0.5f,  0.0f}, {0, 0, 1}, {0, 1}}
        };
        std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };
        return std::make_shared<Mesh>(vertices, indices);
    }

    std::vector<Vec3> positions;
    std::vector<Vec2> texCoords;
    std::vector<Vec3> normals;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            Vec3 v;
            ss >> v.x >> v.y >> v.z;
            positions.push_back(v);
        } else if (prefix == "vt") {
            Vec2 vt;
            ss >> vt.x >> vt.y;
            texCoords.push_back(vt);
        } else if (prefix == "vn") {
            Vec3 vn;
            ss >> vn.x >> vn.y >> vn.z;
            normals.push_back(vn);
        } else if (prefix == "f") {
            std::string vertexStr;
            for (int i = 0; i < 3; ++i) {
                ss >> vertexStr;
                std::replace(vertexStr.begin(), vertexStr.end(), '/', ' ');
                std::stringstream vss(vertexStr);
                uint32_t pIdx, tIdx, nIdx;
                vss >> pIdx;
                
                Vertex v;
                v.Position = positions[pIdx - 1];
                
                if (vss >> tIdx) {
                    v.TexCoord = texCoords[tIdx - 1];
                }
                if (vss >> nIdx) {
                    v.Normal = normals[nIdx - 1];
                }

                vertices.push_back(v);
                indices.push_back((uint32_t)vertices.size() - 1);
            }
        }
    }

    return std::make_shared<Mesh>(vertices, indices);
}

} // namespace MyEngine
