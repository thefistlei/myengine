/******************************************************************************
 * File: TerrainPass.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Heightmap-based terrain rendering implementation
 ******************************************************************************/

#include "TerrainPass.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderBackend.h"
#include "Core/Log.h"
#include "ECS/Entity.h"
#include "ECS/Components.h"
#include <imgui.h>
#include <glad/gl.h>
#include <cmath>

namespace MyEngine {

TerrainPass::TerrainPass() {
    SetPriority(30); // Render after water
}

void TerrainPass::OnCreate(RenderBackend* backend) {
    CreateTerrainMesh();
    CreateTerrainShader();
    ENGINE_INFO("[TerrainPass] Created");
}

void TerrainPass::OnDestroy() {
    m_TerrainMesh.reset();
    m_TerrainShader.reset();
}

float TerrainPass::PerlinNoise(float x, float z) {
    // Improved multi-octave noise with better distribution
    float noise = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxValue = 0.0f;
    
    for (int i = 0; i < (int)m_NoiseOctaves; ++i) {
        // Multiple offset sine waves for more variety
        float n1 = std::sin(x * frequency * 0.1f + 1.5f) * std::cos(z * frequency * 0.1f + 2.3f);
        float n2 = std::sin((x + z) * frequency * 0.15f + 3.7f) * 0.5f;
        float n3 = std::cos((x - z) * frequency * 0.12f + 5.1f) * 0.3f;
        
        // Diagonal waves for more natural look
        float n4 = std::sin((x * 0.7f + z * 1.3f) * frequency * 0.08f) * 0.4f;
        
        // Add ridges for mountain-like features
        float ridge = 1.0f - std::abs(std::sin(x * frequency * 0.05f) * std::cos(z * frequency * 0.05f));
        float n5 = ridge * 0.6f;
        
        noise += (n1 + n2 + n3 + n4 + n5) * amplitude;
        maxValue += amplitude;
        
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }
    
    // Normalize to [-1, 1] range
    noise /= maxValue;
    
    // Apply power curve for more interesting terrain shape
    // Creates flatter plains and sharper peaks
    float sign = (noise >= 0.0f) ? 1.0f : -1.0f;
    noise = sign * std::pow(std::abs(noise), 0.7f);  // Lower power = more variation
    
    return noise;
}

float TerrainPass::GetHeight(int x, int z) {
    float worldX = (x / (float)m_GridSize - 0.5f) * m_TerrainSize;
    float worldZ = (z / (float)m_GridSize - 0.5f) * m_TerrainSize;
    
    float height = PerlinNoise(worldX * m_NoiseScale, worldZ * m_NoiseScale);
    return height * m_MaxHeight;
}

void TerrainPass::CreateTerrainMesh() {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    // Generate heightmap
    m_HeightData.clear();
    m_HeightData.resize((m_GridSize + 1) * (m_GridSize + 1));
    
    // Generate vertices
    for (int z = 0; z <= m_GridSize; ++z) {
        for (int x = 0; x <= m_GridSize; ++x) {
            Vertex vertex;
            
            float height = GetHeight(x, z);
            m_HeightData[z * (m_GridSize + 1) + x] = height;
            
            vertex.Position.x = (x / (float)m_GridSize - 0.5f) * m_TerrainSize;
            vertex.Position.y = height;
            vertex.Position.z = (z / (float)m_GridSize - 0.5f) * m_TerrainSize;
            vertex.TexCoord = Vec2(x / (float)m_GridSize, z / (float)m_GridSize);
            
            // Calculate normal from neighboring heights
            float hL = (x > 0) ? m_HeightData[z * (m_GridSize + 1) + (x - 1)] : height;
            float hR = (x < m_GridSize) ? m_HeightData[z * (m_GridSize + 1) + (x + 1)] : height;
            float hD = (z > 0) ? m_HeightData[(z - 1) * (m_GridSize + 1) + x] : height;
            float hU = (z < m_GridSize) ? m_HeightData[(z + 1) * (m_GridSize + 1) + x] : height;
            
            Vec3 normal;
            normal.x = hL - hR;
            normal.y = 2.0f;
            normal.z = hD - hU;
            normal = normal.Normalized();
            vertex.Normal = normal;
            
            vertices.push_back(vertex);
        }
    }
    
    // Generate indices
    for (int z = 0; z < m_GridSize; ++z) {
        for (int x = 0; x < m_GridSize; ++x) {
            int topLeft = z * (m_GridSize + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (m_GridSize + 1) + x;
            int bottomRight = bottomLeft + 1;
            
            // Triangle 1
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            
            // Triangle 2
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
    
    m_TerrainMesh = std::make_unique<Mesh>(vertices, indices);
    ENGINE_INFO("[TerrainPass] Created terrain mesh: {} vertices, {} triangles", 
                vertices.size(), indices.size() / 3);
}

void TerrainPass::CreateTerrainShader() {
    const char* vertexSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec3 a_Normal;
        layout(location = 2) in vec2 a_TexCoord;
        
        uniform mat4 u_ViewProjection;
        uniform mat4 u_Model;
        
        out vec3 v_WorldPos;
        out vec3 v_Normal;
        out vec2 v_TexCoord;
        out float v_Height;
        
        void main() {
            // Transform position to world space using model matrix
            vec4 worldPos = u_Model * vec4(a_Position, 1.0);
            v_WorldPos = worldPos.xyz;
            
            // Transform normal by model matrix (upper-left 3x3)
            mat3 normalMatrix = mat3(u_Model);
            v_Normal = normalize(normalMatrix * a_Normal);
            
            v_TexCoord = a_TexCoord;
            v_Height = worldPos.y;  // Use world space height
            
            gl_Position = u_ViewProjection * worldPos;
        }
    )";
    
    const char* fragmentSrc = R"(
        #version 330 core
        in vec3 v_WorldPos;
        in vec3 v_Normal;
        in vec2 v_TexCoord;
        in float v_Height;
        
        uniform vec3 u_CameraPos;
        uniform vec3 u_GrassColor;
        uniform vec3 u_RockColor;
        uniform vec3 u_SandColor;
        uniform vec3 u_SunDirection;
        
        out vec4 FragColor;
        
        // Hash function for procedural noise
        float hash(vec2 p) {
            p = fract(p * vec2(123.34, 456.21));
            p += dot(p, p + 45.32);
            return fract(p.x * p.y);
        }
        
        // Procedural noise for detail
        float noise(vec2 p) {
            vec2 i = floor(p);
            vec2 f = fract(p);
            f = f * f * (3.0 - 2.0 * f);
            
            float a = hash(i);
            float b = hash(i + vec2(1.0, 0.0));
            float c = hash(i + vec2(0.0, 1.0));
            float d = hash(i + vec2(1.0, 1.0));
            
            return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
        }
        
        // Tri-planar texture mapping weight
        vec3 getTriplanarWeights(vec3 normal) {
            vec3 blending = abs(normal);
            blending = normalize(max(blending, 0.00001));
            float b = (blending.x + blending.y + blending.z);
            blending /= vec3(b, b, b);
            return blending;
        }
        
        void main() {
            vec3 normal = normalize(v_Normal);
            vec3 viewDir = normalize(u_CameraPos - v_WorldPos);
            
            // Add detail normal perturbation
            float detailNoise1 = noise(v_WorldPos.xz * 10.0) * 2.0 - 1.0;
            float detailNoise2 = noise(v_WorldPos.xz * 20.0) * 2.0 - 1.0;
            vec3 detailNormal = normal + vec3(detailNoise1, 0.0, detailNoise2) * 0.1;
            detailNormal = normalize(detailNormal);
            
            // Calculate slope
            float slope = 1.0 - normal.y;
            
            // Improved height-based material blending with wider transitions
            float sandBlend = smoothstep(2.0, -2.0, v_Height);
            float grassBlend = smoothstep(-2.0, 4.0, v_Height) * smoothstep(14.0, 8.0, v_Height);
            float rockBlend = smoothstep(8.0, 14.0, v_Height);
            
            // Stronger slope-based blending (steep = rock)
            float slopeRock = smoothstep(0.25, 0.55, slope);
            rockBlend = max(rockBlend, slopeRock);
            grassBlend = grassBlend * (1.0 - slopeRock * 0.8);
            sandBlend = sandBlend * (1.0 - slopeRock * 0.9);
            
            // Normalize blending weights
            float totalBlend = sandBlend + grassBlend + rockBlend + 0.001;
            sandBlend /= totalBlend;
            grassBlend /= totalBlend;
            rockBlend /= totalBlend;
            
            // Add more color variation with noise
            vec3 sandColor = u_SandColor * (0.85 + noise(v_WorldPos.xz * 5.0) * 0.3);
            vec3 grassColor = u_GrassColor * (0.7 + noise(v_WorldPos.xz * 8.0) * 0.6);
            vec3 rockColor = u_RockColor * (0.8 + noise(v_WorldPos.xz * 3.0) * 0.4);
            
            // Blend materials
            vec3 baseColor = sandColor * sandBlend + grassColor * grassBlend + rockColor * rockBlend;
            
            // Lighting setup
            vec3 sunDir = normalize(u_SunDirection);
            float NdotL = max(dot(detailNormal, sunDir), 0.0);
            
            // Enhanced diffuse lighting
            float diffuse = NdotL * 0.8 + 0.2;
            
            // Self-shadowing for valleys
            float selfShadow = smoothstep(-8.0, 5.0, v_Height) * 0.4 + 0.6;
            
            // Enhanced specular for wet rocks
            vec3 halfDir = normalize(sunDir + viewDir);
            float specular = pow(max(dot(detailNormal, halfDir), 0.0), 32.0) * rockBlend * 0.4;
            
            // Stronger ambient occlusion based on height
            float ao = smoothstep(-10.0, 15.0, v_Height) * 0.4 + 0.6;
            
            // Enhanced sky light
            float skyLight = max(0.0, normal.y) * 0.5;
            vec3 skyColor = vec3(0.5, 0.6, 0.8);
            
            // Combine lighting with stronger contrast
            float ambient = 0.3;
            vec3 finalColor = baseColor * (ambient * ao + diffuse * 0.7 * selfShadow + skyLight);
            finalColor += skyColor * skyLight * 0.4;
            finalColor += vec3(specular);
            
            // Adjusted distance fog
            float distToCamera = length(u_CameraPos - v_WorldPos);
            float fogFactor = smoothstep(60.0, 200.0, distToCamera);
            vec3 fogColor = vec3(0.7, 0.8, 0.9);
            finalColor = mix(finalColor, fogColor, fogFactor * 0.5);
            
            // Subtle atmospheric perspective
            finalColor = mix(finalColor, fogColor, fogFactor * 0.2);
            
            FragColor = vec4(finalColor, 1.0);
        }
    )";
    
    m_TerrainShader.reset(Shader::Create("TerrainShader", vertexSrc, fragmentSrc));
    ENGINE_INFO("[TerrainPass] Terrain shader created");
}

void TerrainPass::Execute(const SceneView& view, Registry* registry) {
    if (!m_TerrainMesh || !m_TerrainShader) return;
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    if (m_ShowWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    m_TerrainShader->Bind();
    m_TerrainShader->SetMat4("u_ViewProjection", view.GetViewProjectionMatrix());
    m_TerrainShader->SetFloat3("u_CameraPos", view.GetPosition());
    m_TerrainShader->SetFloat3("u_GrassColor", m_GrassColor);
    m_TerrainShader->SetFloat3("u_RockColor", m_RockColor);
    m_TerrainShader->SetFloat3("u_SandColor", m_SandColor);
    m_TerrainShader->SetFloat3("u_SunDirection", Vec3(0.3f, 0.8f, 0.5f));
    
    // Find the TerrainPass entity and get its transform
    Mat4 modelMatrix = Mat4(); // Identity matrix
    bool foundEntity = false;
    if (registry) {
        auto entities = registry->GetEntitiesWith<PassComponent, TransformComponent>();
        for (auto entityID : entities) {
            Entity entity(entityID, registry);
            auto& passComp = entity.GetComponent<PassComponent>();
            // Check if this entity corresponds to this terrain pass
            if (passComp.pass == this) {
                auto& transform = entity.GetComponent<TransformComponent>();
                modelMatrix = transform.localMatrix;
                foundEntity = true;
                
                // Debug: Log transform info every 10 frames
                static int frameCount = 0;
                if (++frameCount % 10 == 0) {
                    ENGINE_INFO("[TerrainPass] Entity found, Position: ({}, {}, {}), Matrix[12-14]: ({}, {}, {})",
                        transform.localPosition.x, transform.localPosition.y, transform.localPosition.z,
                        modelMatrix.m[12], modelMatrix.m[13], modelMatrix.m[14]);
                }
                break;
            }
        }
    }
    
    if (!foundEntity) {
        static bool warned = false;
        if (!warned) {
            ENGINE_WARN("[TerrainPass] Entity not found, using identity matrix");
            warned = true;
        }
    }
    
    // Set model matrix uniform
    m_TerrainShader->SetMat4("u_Model", modelMatrix);
    
    Renderer::DrawMesh(*m_TerrainMesh, modelMatrix);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void TerrainPass::OnGUI() {
    ImGui::Text("Terrain Configuration");
    ImGui::Separator();
    
    bool needsRegenerate = false;
    
    if (ImGui::SliderFloat("Terrain Size", &m_TerrainSize, 50.0f, 500.0f)) {
        needsRegenerate = true;
    }
    if (ImGui::SliderFloat("Max Height", &m_MaxHeight, 1.0f, 50.0f)) {
        needsRegenerate = true;
    }
    if (ImGui::SliderInt("Grid Size", &m_GridSize, 16, 128)) {
        needsRegenerate = true;
    }
    if (ImGui::SliderFloat("Noise Scale", &m_NoiseScale, 0.01f, 0.2f)) {
        needsRegenerate = true;
    }
    if (ImGui::SliderFloat("Octaves", &m_NoiseOctaves, 1.0f, 8.0f)) {
        needsRegenerate = true;
    }
    
    ImGui::Separator();
    ImGui::ColorEdit3("Grass Color", &m_GrassColor.x);
    ImGui::ColorEdit3("Rock Color", &m_RockColor.x);
    ImGui::ColorEdit3("Sand Color", &m_SandColor.x);
    ImGui::Checkbox("Wireframe", &m_ShowWireframe);
    
    if (needsRegenerate || ImGui::Button("Regenerate Terrain")) {
        RegenerateTerrain();
    }
}

void TerrainPass::RegenerateTerrain() {
    CreateTerrainMesh();
    ENGINE_INFO("[TerrainPass] Terrain regenerated");
}

REGISTER_PASS(TerrainPass);

} // namespace MyEngine
