/******************************************************************************
 * File: AnimatedMesh.cpp
 * Description: Animated mesh implementation with bone-weighted vertex buffer
 ******************************************************************************/

#include "AnimatedMesh.h"
#include "Core/Log.h"
#include "Rendering/OpenGL/OpenGLVertexArray.h"
#include "Rendering/OpenGL/OpenGLBuffer.h"
#include <glad/glad.h>

namespace MyEngine {

AnimatedMesh::AnimatedMesh(const std::vector<AnimatedVertex>& vertices, const std::vector<uint32_t>& indices)
    : m_Vertices(vertices)
    , m_Indices(indices)
{
    // Create Vertex Array
    m_VertexArray = std::make_shared<OpenGLVertexArray>();
    
    // Create Vertex Buffer
    auto vertexBuffer = std::make_shared<OpenGLVertexBuffer>(
        static_cast<float*>(static_cast<void*>(const_cast<AnimatedVertex*>(m_Vertices.data()))),
        m_Vertices.size() * sizeof(AnimatedVertex)
    );
    
    // Set vertex layout for animated mesh
    // layout(location = 0) in vec3 pos;
    // layout(location = 1) in vec3 norm;
    // layout(location = 2) in vec2 tex;
    // layout(location = 3) in vec3 tangent;
    // layout(location = 4) in vec3 bitangent;
    // layout(location = 5) in ivec4 boneIds; 
    // layout(location = 6) in vec4 weights;
    
    BufferLayout layout = {
        { ShaderDataType::Float3, "a_Position" },
        { ShaderDataType::Float3, "a_Normal" },
        { ShaderDataType::Float2, "a_TexCoord" },
        { ShaderDataType::Float3, "a_Tangent" },
        { ShaderDataType::Float3, "a_Bitangent" },
        { ShaderDataType::Int4,   "a_BoneIDs" },
        { ShaderDataType::Float4, "a_Weights" }
    };
    
    vertexBuffer->SetLayout(layout);
    m_VertexArray->AddVertexBuffer(vertexBuffer);
    
    // Create Index Buffer
    auto indexBuffer = std::make_shared<OpenGLIndexBuffer>(
        m_Indices.data(),
        m_Indices.size()
    );
    
    m_VertexArray->SetIndexBuffer(indexBuffer);
    
    ENGINE_INFO("AnimatedMesh created: {} vertices, {} indices", m_Vertices.size(), m_Indices.size());
}

} // namespace MyEngine
