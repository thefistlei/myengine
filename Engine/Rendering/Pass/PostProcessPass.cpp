/******************************************************************************
 * File: PostProcessPass.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Post-processing effects implementation
 ******************************************************************************/

#include "PostProcessPass.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderBackend.h"
#include "Core/Log.h"
#include <imgui.h>
#include <glad/gl.h>
#include <cmath>

namespace MyEngine {

PostProcessPass::PostProcessPass() {
    SetPriority(1000); // Render last, after all other passes
}

void PostProcessPass::OnCreate(RenderBackend* backend) {
    ENGINE_INFO("[PostProcessPass] OnCreate called");
    try {
        CreateFullscreenQuad();
        ENGINE_INFO("[PostProcessPass] Fullscreen quad created");
        CreatePostProcessShader();
        ENGINE_INFO("[PostProcessPass] Post-process shader created");

        // Create a 1x1 fallback white texture to avoid black sampling when screen texture isn't bound.
        if (m_FallbackWhiteTex == 0) {
            GLuint tex = 0;
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            unsigned char white[4] = { 255, 255, 255, 255 };
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);
            m_FallbackWhiteTex = tex;
        }

        ENGINE_INFO("[PostProcessPass] Created successfully");
    } catch (const std::exception& e) {
        ENGINE_ERROR("[PostProcessPass] Failed to create: {}", e.what());
    } catch (...) {
        ENGINE_ERROR("[PostProcessPass] Failed to create: unknown error");
    }
}

void PostProcessPass::OnDestroy() {
    m_PostProcessShader.reset();

    if (m_FallbackWhiteTex) {
        glDeleteTextures(1, &m_FallbackWhiteTex);
        m_FallbackWhiteTex = 0;
    }

    if (m_QuadVAO) {
        glDeleteVertexArrays(1, &m_QuadVAO);
        m_QuadVAO = 0;
    }
    if (m_QuadVBO) {
        glDeleteBuffers(1, &m_QuadVBO);
        m_QuadVBO = 0;
    }
}

void PostProcessPass::CreateFullscreenQuad() {
    // Fullscreen quad vertices (position + texcoord)
    float quadVertices[] = {
        // Positions      // TexCoords
        -1.0f,  1.0f,     0.0f, 1.0f,
        -1.0f, -1.0f,     0.0f, 0.0f,
         1.0f, -1.0f,     1.0f, 0.0f,

        -1.0f,  1.0f,     0.0f, 1.0f,
         1.0f, -1.0f,     1.0f, 0.0f,
         1.0f,  1.0f,     1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_QuadVAO);
    glGenBuffers(1, &m_QuadVBO);

    glBindVertexArray(m_QuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // TexCoord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    ENGINE_INFO("[PostProcessPass] Created fullscreen quad");
}

void PostProcessPass::CreatePostProcessShader() {
    const char* vertexSrc = R"(
        #version 330 core
        layout(location = 0) in vec2 a_Position;
        layout(location = 1) in vec2 a_TexCoord;

        out vec2 v_TexCoord;

        void main() {
            v_TexCoord = a_TexCoord;
            gl_Position = vec4(a_Position, 0.0, 1.0);
        }
    )";

    const char* fragmentSrc = R"(
        #version 330 core
        in vec2 v_TexCoord;

        uniform sampler2D u_ScreenTexture;
        uniform float u_Time;

        // Effect toggles
        uniform bool u_EnableBloom;
        uniform bool u_EnableVignette;
        uniform bool u_EnableChromaticAberration;
        uniform bool u_EnableFilmGrain;

        // Color grading
        uniform float u_Exposure;
        uniform float u_Contrast;
        uniform float u_Saturation;
        uniform float u_Brightness;

        // Bloom
        uniform float u_BloomIntensity;
        uniform float u_BloomThreshold;

        // Vignette
        uniform float u_VignetteIntensity;
        uniform float u_VignetteRadius;

        // Chromatic aberration
        uniform float u_ChromaticAberrationStrength;

        // Film grain
        uniform float u_FilmGrainIntensity;

        out vec4 FragColor;

        // Random function for film grain
        float random(vec2 co) {
            return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
        }

        // Simple blur for bloom approximation
        vec3 sampleBloom(vec2 uv) {
            vec3 color = vec3(0.0);
            float kernel[9] = float[](1.0, 2.0, 1.0, 2.0, 4.0, 2.0, 1.0, 2.0, 1.0);
            vec2 offsets[9] = vec2[](
                vec2(-1, -1), vec2(0, -1), vec2(1, -1),
                vec2(-1,  0), vec2(0,  0), vec2(1,  0),
                vec2(-1,  1), vec2(0,  1), vec2(1,  1)
            );

            float pixelSize = 0.002;
            for (int i = 0; i < 9; i++) {
                vec3 sample = texture(u_ScreenTexture, uv + offsets[i] * pixelSize).rgb;
                color += sample * kernel[i] / 16.0;
            }

            return color;
        }

        void main() {
            vec2 uv = v_TexCoord;
            vec3 color;

            // Chromatic aberration
            if (u_EnableChromaticAberration) {
                vec2 dir = uv - vec2(0.5);
                float dist = length(dir);
                dir = normalize(dir);

                color.r = texture(u_ScreenTexture, uv + dir * u_ChromaticAberrationStrength * dist).r;
                color.g = texture(u_ScreenTexture, uv).g;
                color.b = texture(u_ScreenTexture, uv - dir * u_ChromaticAberrationStrength * dist).b;
            } else {
                color = texture(u_ScreenTexture, uv).rgb;
            }

            // Bloom
            if (u_EnableBloom) {
                vec3 bloom = sampleBloom(uv);
                bloom = max(bloom - u_BloomThreshold, 0.0);
                color += bloom * u_BloomIntensity;
            }

            // Exposure and tone mapping
            color = vec3(1.0) - exp(-color * u_Exposure);

            // Brightness
            color += u_Brightness;

            // Contrast
            color = (color - 0.5) * u_Contrast + 0.5;

            // Saturation
            float luminance = dot(color, vec3(0.299, 0.587, 0.114));
            color = mix(vec3(luminance), color, u_Saturation);

            // Vignette
            if (u_EnableVignette) {
                vec2 center = uv - vec2(0.5);
                float dist = length(center);
                float vignette = smoothstep(u_VignetteRadius, u_VignetteRadius - 0.3, dist);
                color = mix(color * 0.3, color, vignette * (1.0 - u_VignetteIntensity) + u_VignetteIntensity);
            }

            // Film grain
            if (u_EnableFilmGrain) {
                float grain = random(uv * u_Time) * 2.0 - 1.0;
                color += grain * u_FilmGrainIntensity;
            }

            // Clamp to valid range
            color = clamp(color, 0.0, 1.0);

            FragColor = vec4(color, 1.0);
        }
    )";

    m_PostProcessShader.reset(Shader::Create("PostProcessShader", vertexSrc, fragmentSrc));
    ENGINE_INFO("[PostProcessPass] Post-process shader created");
}

void PostProcessPass::Execute(const SceneView& view, Registry* registry) {
    if (!m_PostProcessShader || !m_QuadVAO) return;

    m_Time += 0.016f; // Approximate 60 FPS

    // Save GL state (minimal set to avoid state leakage causing black regions)
    GLboolean wasDepthTest = glIsEnabled(GL_DEPTH_TEST);
    GLboolean wasCullFace  = glIsEnabled(GL_CULL_FACE);
    GLboolean wasBlend     = glIsEnabled(GL_BLEND);
    GLboolean oldDepthMask;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &oldDepthMask);

    GLint oldDepthFunc = GL_LESS;
    glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFunc);

    GLint oldProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);

    GLint oldVAO = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVAO);

    GLint oldActiveTex = 0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &oldActiveTex);

    GLint oldTex2D = 0;
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTex2D);

    // Setup for fullscreen
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    m_PostProcessShader->Bind();

    // Bind screen texture (TODO: replace with real framebuffer color attachment).
    // Fallback to a white texture to avoid sampling black/undefined.
    glActiveTexture(GL_TEXTURE0);
    if (m_ScreenColorTexture != 0) {
        glBindTexture(GL_TEXTURE_2D, m_ScreenColorTexture);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_FallbackWhiteTex);
    }

    // Set uniforms
    m_PostProcessShader->SetInt("u_ScreenTexture", 0);
    m_PostProcessShader->SetFloat("u_Time", m_Time);

    m_PostProcessShader->SetBool("u_EnableBloom", m_EnableBloom);
    m_PostProcessShader->SetBool("u_EnableVignette", m_EnableVignette);
    m_PostProcessShader->SetBool("u_EnableChromaticAberration", m_EnableChromaticAberration);
    m_PostProcessShader->SetBool("u_EnableFilmGrain", m_EnableFilmGrain);

    m_PostProcessShader->SetFloat("u_Exposure", m_Exposure);
    m_PostProcessShader->SetFloat("u_Contrast", m_Contrast);
    m_PostProcessShader->SetFloat("u_Saturation", m_Saturation);
    m_PostProcessShader->SetFloat("u_Brightness", m_Brightness);

    m_PostProcessShader->SetFloat("u_BloomIntensity", m_BloomIntensity);
    m_PostProcessShader->SetFloat("u_BloomThreshold", m_BloomThreshold);

    m_PostProcessShader->SetFloat("u_VignetteIntensity", m_VignetteIntensity);
    m_PostProcessShader->SetFloat("u_VignetteRadius", m_VignetteRadius);

    m_PostProcessShader->SetFloat("u_ChromaticAberrationStrength", m_ChromaticAberrationStrength);
    m_PostProcessShader->SetFloat("u_FilmGrainIntensity", m_FilmGrainIntensity);

    glBindVertexArray(m_QuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Restore texture bindings
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, oldTex2D);
    glActiveTexture(oldActiveTex);

    // Restore shader program/VAO
    glUseProgram(oldProgram);
    glBindVertexArray(oldVAO);

    // Restore depth/cull/blend state
    if (wasDepthTest) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (wasCullFace)  glEnable(GL_CULL_FACE);  else glDisable(GL_CULL_FACE);
    if (wasBlend)     glEnable(GL_BLEND);      else glDisable(GL_BLEND);
    glDepthMask(oldDepthMask ? GL_TRUE : GL_FALSE);
    glDepthFunc(oldDepthFunc);
}

void PostProcessPass::OnGUI() {
    ImGui::Text("Post-Processing Effects");
    ImGui::Separator();

    // Effect toggles
    ImGui::Checkbox("Bloom", &m_EnableBloom);
    ImGui::Checkbox("Vignette", &m_EnableVignette);
    ImGui::Checkbox("Chromatic Aberration", &m_EnableChromaticAberration);
    ImGui::Checkbox("Film Grain", &m_EnableFilmGrain);

    ImGui::Separator();
    ImGui::Text("Color Grading");
    ImGui::SliderFloat("Exposure", &m_Exposure, 0.0f, 3.0f);
    ImGui::SliderFloat("Contrast", &m_Contrast, 0.0f, 2.0f);
    ImGui::SliderFloat("Saturation", &m_Saturation, 0.0f, 2.0f);
    ImGui::SliderFloat("Brightness", &m_Brightness, -0.5f, 0.5f);

    if (m_EnableBloom) {
        ImGui::Separator();
        ImGui::Text("Bloom Settings");
        ImGui::SliderFloat("Bloom Intensity", &m_BloomIntensity, 0.0f, 1.0f);
        ImGui::SliderFloat("Bloom Threshold", &m_BloomThreshold, 0.0f, 2.0f);
    }

    if (m_EnableVignette) {
        ImGui::Separator();
        ImGui::Text("Vignette Settings");
        ImGui::SliderFloat("Vignette Intensity", &m_VignetteIntensity, 0.0f, 1.0f);
        ImGui::SliderFloat("Vignette Radius", &m_VignetteRadius, 0.1f, 1.5f);
    }

    if (m_EnableChromaticAberration) {
        ImGui::Separator();
        ImGui::Text("Chromatic Aberration");
        ImGui::SliderFloat("CA Strength", &m_ChromaticAberrationStrength, 0.0f, 0.01f);
    }

    if (m_EnableFilmGrain) {
        ImGui::Separator();
        ImGui::Text("Film Grain");
        ImGui::SliderFloat("Grain Intensity", &m_FilmGrainIntensity, 0.0f, 0.2f);
    }

    if (ImGui::Button("Reset to Defaults")) {
        m_Exposure = 1.0f;
        m_Contrast = 1.0f;
        m_Saturation = 1.0f;
        m_Brightness = 0.0f;
        m_BloomIntensity = 0.3f;
        m_BloomThreshold = 1.0f;
        m_VignetteIntensity = 0.3f;
        m_VignetteRadius = 0.8f;
        m_ChromaticAberrationStrength = 0.002f;
        m_FilmGrainIntensity = 0.05f;
    }
}

REGISTER_PASS(PostProcessPass);

} // namespace MyEngine
