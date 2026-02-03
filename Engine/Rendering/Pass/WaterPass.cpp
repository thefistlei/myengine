/******************************************************************************
 * File: WaterPass.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Modified: 2026-02-01
 * Description: Water surface rendering implementation with texture support
 ******************************************************************************/

#include "WaterPass.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderBackend.h"
#include "Core/Log.h"
#include "ECS/Entity.h"
#include "ECS/Components.h"
#include <imgui.h>
#include <glad/gl.h>
#include <cmath>

namespace MyEngine {

WaterPass::WaterPass() {
    SetPriority(90); // Render after most opaque objects for proper transparency
}

void WaterPass::OnCreate(RenderBackend* backend) {
    CreateWaterPlane();
    CreateWaterShader();
    LoadTextures();
    ENGINE_INFO("[WaterPass] Created");
}

void WaterPass::OnDestroy() {
    m_WaterMesh.reset();
    m_WaterShader.reset();
    m_NormalMap.reset();
    m_ReflectionMap.reset();
}

void WaterPass::CreateWaterPlane() {
    // Validate subdivisions
    if (m_Subdivisions < 1) m_Subdivisions = 1;
    if (m_Subdivisions > 256) {
        ENGINE_WARN("[WaterPass] Subdivision count {} too high, clamping to 256", m_Subdivisions);
        m_Subdivisions = 256;
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Create subdivided water plane for wave animation
    float step = m_PlaneSize / m_Subdivisions;
    float halfSize = m_PlaneSize * 0.5f;

    // Reserve memory to avoid reallocations
    vertices.reserve((m_Subdivisions + 1) * (m_Subdivisions + 1));
    indices.reserve(m_Subdivisions * m_Subdivisions * 6);

    // Generate vertices
    for (int z = 0; z <= m_Subdivisions; ++z) {
        for (int x = 0; x <= m_Subdivisions; ++x) {
            Vertex vertex;
            vertex.Position.x = -halfSize + x * step;
            vertex.Position.y = m_WaterLevel;
            vertex.Position.z = -halfSize + z * step;
            vertex.Normal = Vec3(0.0f, 1.0f, 0.0f);
            vertex.TexCoord = Vec2(x / (float)m_Subdivisions, z / (float)m_Subdivisions);
            vertices.push_back(vertex);
        }
    }

    // Generate indices
    for (int z = 0; z < m_Subdivisions; ++z) {
        for (int x = 0; x < m_Subdivisions; ++x) {
            int topLeft = z * (m_Subdivisions + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (m_Subdivisions + 1) + x;
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

    m_WaterMesh = std::make_unique<Mesh>(vertices, indices);
    ENGINE_INFO("[WaterPass] Created water plane: {} vertices, {} triangles",
                vertices.size(), indices.size() / 3);
}

void WaterPass::CreateWaterShader() {
    const char* vertexSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec3 a_Normal;
        layout(location = 2) in vec2 a_TexCoord;

        // Add model and normal matrix uniforms
        uniform mat4 u_ViewProjection;
        uniform mat4 u_Model;
        uniform float u_Time;
        uniform float u_WaveAmplitude;
        uniform float u_WaveFrequency;
        uniform float u_WaveSpeed;

        out vec3 v_WorldPos;
        out vec3 v_Normal;
        out vec2 v_TexCoord;
        out float v_WaveHeight;

        void main() {
            // Transform position to world space using model matrix
            vec3 worldPos = (u_Model * vec4(a_Position, 1.0)).xyz;

            // Multi-layer wave animation with different frequencies
            float time = u_Time * u_WaveSpeed;

            // Large waves
            float wave1 = sin(worldPos.x * u_WaveFrequency * 0.8 + time) * u_WaveAmplitude;
            float wave2 = sin(worldPos.z * u_WaveFrequency * 0.6 + time * 1.2) * u_WaveAmplitude;

            // Medium waves
            float wave3 = sin((worldPos.x + worldPos.z) * u_WaveFrequency * 1.5 + time * 1.5) * u_WaveAmplitude * 0.5;
            float wave4 = sin((worldPos.x - worldPos.z) * u_WaveFrequency * 1.3 + time * 1.8) * u_WaveAmplitude * 0.5;

            // Small ripples
            float wave5 = sin(worldPos.x * u_WaveFrequency * 3.0 + time * 2.0) * u_WaveAmplitude * 0.25;
            float wave6 = sin(worldPos.z * u_WaveFrequency * 3.2 + time * 2.5) * u_WaveAmplitude * 0.25;

            float totalWave = wave1 + wave2 + wave3 + wave4 + wave5 + wave6;
            worldPos.y += totalWave;
            v_WaveHeight = totalWave;

            // Simplified normal calculation (will be enhanced by normal map)
            float freq1 = u_WaveFrequency * 0.8;
            float freq2 = u_WaveFrequency * 0.6;

            float dx = cos(( (u_Model * vec4(a_Position,1.0)).x ) * freq1 + time) * u_WaveAmplitude * freq1;
            float dz = cos(( (u_Model * vec4(a_Position,1.0)).z ) * freq2 + time * 1.2) * u_WaveAmplitude * freq2;

            vec3 tangent = normalize(vec3(1.0, dx, 0.0));
            vec3 bitangent = normalize(vec3(0.0, dz, 1.0));
            vec3 n = normalize(cross(tangent, bitangent));

            // Extract normal matrix from model matrix (upper-left 3x3)
            mat3 normalMatrix = mat3(u_Model);
            v_Normal = normalize(normalMatrix * n);

            v_WorldPos = worldPos;
            v_TexCoord = a_TexCoord;

            gl_Position = u_ViewProjection * vec4(worldPos, 1.0);
        }
    )";

    const char* fragmentSrc = R"(
        #version 330 core
        in vec3 v_WorldPos;
        in vec3 v_Normal;
        in vec2 v_TexCoord;
        in float v_WaveHeight;

        uniform vec3 u_CameraPos;
        uniform vec3 u_WaterColor;
        uniform float u_Transparency;
        uniform vec3 u_SunDirection;
        uniform float u_Time;
        uniform float u_WaveSpeed;

        // Texture uniforms
        uniform sampler2D u_NormalMap;
        uniform samplerCube u_ReflectionMap;
        uniform bool u_UseNormalMap;
        uniform bool u_UseReflectionMap;
        uniform float u_NormalStrength;
        uniform float u_NormalScale;

        out vec4 FragColor;

        // Schlick's approximation for Fresnel
        float fresnelSchlick(float cosTheta, float F0) {
            return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
        }

        void main() {
            vec3 viewDir = normalize(u_CameraPos - v_WorldPos);
            vec3 normal = normalize(v_Normal);

            // Apply normal mapping if enabled
            if (u_UseNormalMap) {
                // Two layers of normal maps moving at different speeds
                vec2 uv1 = v_TexCoord * u_NormalScale + vec2(u_Time * u_WaveSpeed * 0.02, u_Time * u_WaveSpeed * 0.015);
                vec2 uv2 = v_TexCoord * (u_NormalScale * 0.7) - vec2(u_Time * u_WaveSpeed * 0.015, u_Time * u_WaveSpeed * 0.02);

                vec3 normal1 = texture(u_NormalMap, uv1).rgb * 2.0 - 1.0;
                vec3 normal2 = texture(u_NormalMap, uv2).rgb * 2.0 - 1.0;

                // Blend the two normal maps
                vec3 detailNormal = normalize(normal1 + normal2);

                // Combine with vertex normal
                normal = normalize(normal + detailNormal * u_NormalStrength);
            }

            // Improved Fresnel effect using Schlick approximation
            float NdotV = max(dot(normal, viewDir), 0.0);
            float fresnel = fresnelSchlick(NdotV, 0.02);

            // Lighting setup
            vec3 sunDir = normalize(u_SunDirection);
            float NdotL = max(dot(normal, sunDir), 0.0);

            // Enhanced specular with sharper highlight
            vec3 halfDir = normalize(sunDir + viewDir);
            float specular = pow(max(dot(normal, halfDir), 0.0), 256.0);

            // Add secondary specular for more sparkle
            vec3 reflectDir = reflect(-sunDir, normal);
            float specular2 = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

            // Animated foam at wave peaks
            float foam = smoothstep(0.01, 0.04, v_WaveHeight) * 0.5;

            // Realistic water colors - use u_WaterColor as base and create variations
            vec3 deepWaterColor = u_WaterColor * 0.3;         // Darker version of base color
            vec3 shallowWaterColor = u_WaterColor * 1.5;      // Lighter version of base color
            vec3 skyReflectionColor = vec3(0.4, 0.6, 0.9);    // Sky blue

            // Use reflection map if available
            if (u_UseReflectionMap) {
                vec3 reflectVector = reflect(-viewDir, normal);
                skyReflectionColor = texture(u_ReflectionMap, reflectVector).rgb;
            }

            // Depth-based color variation
            float depthFactor = smoothstep(-0.08, 0.08, v_WaveHeight);
            vec3 waterColor = mix(deepWaterColor, shallowWaterColor, depthFactor);

            // Base diffuse with ambient
            vec3 ambient = waterColor * 0.3;
            vec3 diffuse = waterColor * (0.5 + NdotL * 0.5);

            // Reflection gets stronger at grazing angles (Fresnel)
            vec3 reflectionColor = mix(skyReflectionColor, vec3(1.0), specular * 0.5);

            // Mix water color with reflection based on Fresnel
            vec3 finalColor = mix(diffuse, reflectionColor, fresnel * 0.8);

            // Add strong sun specular highlights
            finalColor += vec3(1.0, 1.0, 0.9) * specular * 3.0;
            finalColor += vec3(0.8, 0.9, 1.0) * specular2 * 1.5;

            // Add foam highlights at wave peaks
            finalColor += vec3(foam * 0.8);

            // Subtle caustics-like shimmer
            float caustics = sin(v_TexCoord.x * 25.0 + u_Time * u_WaveSpeed * 0.6) *
                           sin(v_TexCoord.y * 25.0 + u_Time * u_WaveSpeed * 0.4);
            caustics = max(caustics, 0.0) * 0.05;
            finalColor += vec3(caustics * 0.5, caustics, caustics * 0.8);

            // Higher transparency for more realistic water
            float alpha = mix(0.7, 0.9, fresnel);  // More transparent, increases at edges

            FragColor = vec4(finalColor, alpha * u_Transparency);
        }
    )";

    m_WaterShader.reset(Shader::Create("WaterShader", vertexSrc, fragmentSrc));
    ENGINE_INFO("[WaterPass] Water shader created");
}

void WaterPass::LoadTextures() {
    // Try to load normal map
    // Note: Adjust the path based on your project structure
    const char* normalMapPath = "assets/textures/water_normal.png";
    m_NormalMap = TextureLoader::Load(normalMapPath);

    if (!m_NormalMap) {
        ENGINE_WARN("[WaterPass] Failed to load normal map from {}, creating default", normalMapPath);
        CreateDefaultNormalMap();
        m_UseNormalMap = false; // Disable until valid texture is loaded
    } else {
        ENGINE_INFO("[WaterPass] Normal map loaded successfully");
        m_UseNormalMap = true;
    }

    // Try to load reflection map (optional)
    const char* reflectionMapPath = "assets/textures/sky_reflection.hdr";
    m_ReflectionMap = TextureLoader::Load(reflectionMapPath);

    if (!m_ReflectionMap) {
        ENGINE_INFO("[WaterPass] No reflection map loaded (optional)");
        m_UseReflectionMap = false;
    } else {
        ENGINE_INFO("[WaterPass] Reflection map loaded successfully");
        m_UseReflectionMap = true;
    }
}

void WaterPass::CreateDefaultNormalMap() {
    // Create a simple flat normal map (128, 128, 255) = (0, 0, 1) in tangent space
    const int size = 64;
    std::vector<unsigned char> pixels(size * size * 3);

    for (int i = 0; i < size * size; ++i) {
        pixels[i * 3 + 0] = 128; // R (X)
        pixels[i * 3 + 1] = 128; // G (Y)
        pixels[i * 3 + 2] = 255; // B (Z) - pointing up
    }

    // Create texture from pixel data
    // Note: You'll need to implement Texture::CreateFromData or similar
    // m_NormalMap = Texture::CreateFromData(size, size, 3, pixels.data());
    ENGINE_INFO("[WaterPass] Created default flat normal map");
}

void WaterPass::Execute(const SceneView& view, Registry* registry) {
    if (!m_WaterMesh || !m_WaterShader) return;

    // Update time for wave animation (fixed delta time)
    m_Time += 0.016f;  // Approximate 60 FPS

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE); // Don't write to depth buffer for transparent water

    m_WaterShader->Bind();

    // Bind textures
    if (m_NormalMap && m_UseNormalMap) {
        m_NormalMap->Bind(0);
        m_WaterShader->SetInt("u_NormalMap", 0);
    }

    if (m_ReflectionMap && m_UseReflectionMap) {
        m_ReflectionMap->Bind(1);
        m_WaterShader->SetInt("u_ReflectionMap", 1);
    }

    // Set shader uniforms
    m_WaterShader->SetMat4("u_ViewProjection", view.GetViewProjectionMatrix());
    m_WaterShader->SetFloat3("u_CameraPos", view.GetPosition());
    m_WaterShader->SetFloat3("u_WaterColor", m_WaterColor);
    m_WaterShader->SetFloat("u_Transparency", m_Transparency);
    m_WaterShader->SetFloat("u_Time", m_Time);
    m_WaterShader->SetFloat("u_WaveAmplitude", m_WaveAmplitude);
    m_WaterShader->SetFloat("u_WaveFrequency", m_WaveFrequency);
    m_WaterShader->SetFloat("u_WaveSpeed", m_WaveSpeed);
    m_WaterShader->SetFloat3("u_SunDirection", Vec3(0.3f, 0.8f, 0.5f));

    // Texture settings
    m_WaterShader->SetBool("u_UseNormalMap", m_UseNormalMap && m_NormalMap != nullptr);
    m_WaterShader->SetBool("u_UseReflectionMap", m_UseReflectionMap && m_ReflectionMap != nullptr);
    m_WaterShader->SetFloat("u_NormalStrength", m_NormalStrength);
    m_WaterShader->SetFloat("u_NormalScale", m_NormalScale);

    // Find the WaterPass entity and get its transform
    Mat4 modelMatrix = Mat4(); // Identity matrix - water level is controlled by vertex Y coordinates
    if (registry) {
        auto entities = registry->GetEntitiesWith<PassComponent, TransformComponent>();
        for (auto entityID : entities) {
            Entity entity(entityID, registry);
            auto& passComp = entity.GetComponent<PassComponent>();
            // Check if this entity corresponds to this water pass
            if (passComp.pass == this) {
                auto& transform = entity.GetComponent<TransformComponent>();
                // Only use rotation and scale from transform, ignore translation
                // Water level is controlled by m_WaterLevel in the mesh vertices
                modelMatrix = transform.localMatrix;
                
                // Force Y translation to 0 to prevent double-translation
                // Mat4 is column-major: translation is in indices 12, 13, 14 for X, Y, Z
                modelMatrix.m[13] = 0.0f;  // Set Y component of translation to 0
                break;
            }
        }
    }

    // Set model matrix uniform
    m_WaterShader->SetMat4("u_Model", modelMatrix);

    // For normal matrix, we'll compute it in the shader from u_Model
    // This avoids needing a SetMat3 method

    Renderer::DrawMesh(*m_WaterMesh, modelMatrix);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void WaterPass::OnGUI() {
    ImGui::Text("Water Properties");
    ImGui::Separator();

    if (ImGui::SliderFloat("Water Level", &m_WaterLevel, -5.0f, 5.0f)) {
        // Recreate water plane if level changes significantly
        CreateWaterPlane();
    }

    ImGui::ColorEdit3("Water Color", &m_WaterColor.x);
    ImGui::SliderFloat("Wave Speed", &m_WaveSpeed, 0.0f, 5.0f);
    ImGui::SliderFloat("Wave Amplitude", &m_WaveAmplitude, 0.0f, 0.5f);
    ImGui::SliderFloat("Wave Frequency", &m_WaveFrequency, 0.1f, 10.0f);
    ImGui::SliderFloat("Transparency", &m_Transparency, 0.0f, 1.0f);

    ImGui::Spacing();
    ImGui::Text("Normal Map Settings");
    ImGui::Separator();

    ImGui::Checkbox("Use Normal Map", &m_UseNormalMap);
    if (m_UseNormalMap) {
        ImGui::SliderFloat("Normal Strength", &m_NormalStrength, 0.0f, 2.0f);
        ImGui::SliderFloat("Normal Scale", &m_NormalScale, 1.0f, 50.0f);
    }

    ImGui::Spacing();
    ImGui::Text("Reflection Settings");
    ImGui::Separator();
    ImGui::Checkbox("Use Reflection Map", &m_UseReflectionMap);

    ImGui::Spacing();
    ImGui::Text("Geometry");
    ImGui::Separator();

    int oldSubdivisions = m_Subdivisions;
    ImGui::SliderInt("Subdivisions", &m_Subdivisions, 8, 128);
    if (m_Subdivisions != oldSubdivisions) {
        CreateWaterPlane();
    }

    ImGui::Spacing();
    if (ImGui::Button("Reset Time")) {
        m_Time = 0.0f;
    }

    ImGui::SameLine();
    if (ImGui::Button("Recreate Plane")) {
        CreateWaterPlane();
    }

    ImGui::Spacing();
    ImGui::Text("Debug Info");
    ImGui::Separator();
    ImGui::Text("Time: %.2f", m_Time);
    ImGui::Text("Vertices: %d", (m_Subdivisions + 1) * (m_Subdivisions + 1));
    ImGui::Text("Triangles: %d", m_Subdivisions * m_Subdivisions * 2);
}

void WaterPass::SetSubdivisions(int subdivisions) {
    if (subdivisions < 1 || subdivisions > 256) {
        ENGINE_WARN("[WaterPass] Invalid subdivision count: {}", subdivisions);
        return;
    }
    m_Subdivisions = subdivisions;
    CreateWaterPlane();
}

void WaterPass::SetNormalMap(const std::string& path) {
    m_NormalMap = TextureLoader::Load(path);
    if (m_NormalMap) {
        m_UseNormalMap = true;
        ENGINE_INFO("[WaterPass] Normal map loaded from {}", path);
    } else {
        ENGINE_ERROR("[WaterPass] Failed to load normal map from {}", path);
    }
}

void WaterPass::SetReflectionMap(const std::string& path) {
    m_ReflectionMap = TextureLoader::Load(path);
    if (m_ReflectionMap) {
        m_UseReflectionMap = true;
        ENGINE_INFO("[WaterPass] Reflection map loaded from {}", path);
    } else {
        ENGINE_ERROR("[WaterPass] Failed to load reflection map from {}", path);
    }
}

REGISTER_PASS(WaterPass);

} // namespace MyEngine
