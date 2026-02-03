/******************************************************************************
 * File: SkyPass.cpp
 * Author: AI Assistant
 * Created: 2026-01-31
 * Description: Sky pass implementation
 ******************************************************************************/

#include "SkyPass.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderBackend.h"
#include "Rendering/OpenGL/OpenGLShader.h"
#include "Core/Log.h"
#include <imgui.h>
#include <glad/gl.h>
#include <cmath>

namespace MyEngine {

void SkyPass::OnCreate(RenderBackend* backend) {
    m_Backend = backend;

    CreateSkySphere();
    CreateSkyShader();

    ENGINE_INFO("[SkyPass] Created");
}

void SkyPass::OnDestroy() {
    m_SkyMesh.reset();
    if (m_Shader) {
        delete m_Shader;
        m_Shader = nullptr;
    }
    ENGINE_INFO("[SkyPass] Destroyed");
}

void SkyPass::Execute(const SceneView& view, Registry* registry) {
    if (!m_Shader || !m_SkyMesh) return;

    // 保存并在渲染后恢复面剔除状态，避免影响其它 pass 导致黑块
    GLboolean wasCull = glIsEnabled(GL_CULL_FACE);
    if (wasCull) {
        glDisable(GL_CULL_FACE);
    }

    // Disable depth writing (but keep depth test)
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    // glDisable(GL_CULL_FACE); // 不再直接调用（已通过 wasCull 控制）

    // Bind shader
    m_Shader->Bind();

    // Remove translation from view matrix (keep rotation only)
    Mat4 viewNoTranslation = view.GetViewMatrix();
    viewNoTranslation.m[12] = 0.0f;
    viewNoTranslation.m[13] = 0.0f;
    viewNoTranslation.m[14] = 0.0f;

    Mat4 viewProj = view.GetProjectionMatrix() * viewNoTranslation;
    m_Shader->SetMat4("u_ViewProjection", viewProj);
    // 上传去除平移的 view 矩阵到新 uniform，以在顶点着色器中计算方向向量
    m_Shader->SetMat4("u_View", viewNoTranslation);

    // Set sky parameters
    m_Shader->SetFloat3("u_SunDirection", m_SunDirection);
    m_Shader->SetFloat("u_SunIntensity", m_SunIntensity);
    m_Shader->SetFloat3("u_SkyColor", m_SkyColor);
    m_Shader->SetFloat3("u_HorizonColor", m_HorizonColor);
    m_Shader->SetFloat3("u_GroundColor", m_GroundColor);

    // Render sky sphere
    Renderer::DrawMesh(*m_SkyMesh, Mat4());

    // Restore depth writing
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    // 恢复原来的面剔除状态
    if (wasCull) {
        glEnable(GL_CULL_FACE);
    }
}

void SkyPass::OnGUI() {
    ImGui::Text("Sky Parameters");
    ImGui::SliderFloat("Sun Intensity", &m_SunIntensity, 0.0f, 20.0f);
    ImGui::SliderFloat3("Sun Direction", &m_SunDirection.x, -1.0f, 1.0f);
    m_SunDirection = m_SunDirection.Normalized();

    ImGui::ColorEdit3("Sky Color", &m_SkyColor.x);
    ImGui::ColorEdit3("Horizon Color", &m_HorizonColor.x);
    ImGui::ColorEdit3("Ground Color", &m_GroundColor.x);
}

void SkyPass::CreateSkySphere() {
    // Create a simple sky sphere (inverted cube or sphere)
    // For simplicity, using a large cube

    const int segments = 32;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Generate sphere vertices
    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * 3.14159265f / segments;
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);

        for (int lon = 0; lon <= segments; ++lon) {
            float phi = lon * 2.0f * 3.14159265f / segments;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            Vertex vertex;
            vertex.Position.x = cosPhi * sinTheta;
            vertex.Position.y = cosTheta;
            vertex.Position.z = sinPhi * sinTheta;

            vertex.Normal = vertex.Position;
            vertex.TexCoord = Vec2(static_cast<float>(lon) / segments, static_cast<float>(lat) / segments);

            vertices.push_back(vertex);
        }
    }

    // Generate indices
    for (int lat = 0; lat < segments; ++lat) {
        for (int lon = 0; lon < segments; ++lon) {
            int first = (lat * (segments + 1)) + lon;
            int second = first + segments + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    m_SkyMesh = std::make_unique<Mesh>(vertices, indices);
    ENGINE_INFO("[SkyPass] Created sky sphere: {} vertices, {} triangles",
                vertices.size(), indices.size() / 3);
}

void SkyPass::CreateSkyShader() {
    // Simple procedural sky shader
    const char* vertexSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec3 a_Normal;
        layout(location = 2) in vec2 a_TexCoord;

        uniform mat4 u_ViewProjection;
        // 新增：只含旋转的 view 矩阵（去除平移），用于计算方向向量
        uniform mat4 u_View;

        out vec3 v_WorldPos;

        void main() {
            // 使用 u_View（rotation-only）将顶点位置转为方向向量（w=0）
            v_WorldPos = (u_View * vec4(a_Position, 0.0)).xyz;

            gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
            gl_Position = gl_Position.xyww; // Depth = 1.0
        }
    )";

    const char* fragmentSrc = R"(
        #version 330 core

        in vec3 v_WorldPos;

        uniform vec3 u_SunDirection;
        uniform float u_SunIntensity;
        uniform vec3 u_SkyColor;
        uniform vec3 u_HorizonColor;
        uniform vec3 u_GroundColor;

        out vec4 FragColor;

        void main() {
            vec3 viewDir = normalize(v_WorldPos);

            // Sky gradient based on Y coordinate
            float t = viewDir.y * 0.5 + 0.5; // Remap to [0, 1]

            vec3 skyGradient;
            if (viewDir.y > 0.0) {
                // Above horizon: blend sky to horizon
                skyGradient = mix(u_HorizonColor, u_SkyColor, t);
            } else {
                // Below horizon: blend horizon to ground
                skyGradient = mix(u_HorizonColor, u_GroundColor, -viewDir.y);
            }

            // Sun highlight
            float sunDot = max(dot(viewDir, normalize(u_SunDirection)), 0.0);
            float sun = pow(sunDot, 256.0) * u_SunIntensity;

            vec3 finalColor = skyGradient + vec3(sun);

            FragColor = vec4(finalColor, 1.0);
        }
    )";

    m_Shader = new OpenGLShader("SkyShader", vertexSrc, fragmentSrc);
    ENGINE_INFO("[SkyPass] Sky shader created");
}

// Auto-register this pass
REGISTER_PASS(SkyPass);

} // namespace MyEngine
