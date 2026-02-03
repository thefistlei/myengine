/******************************************************************************
 * File: EditorLayer.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Editor layer implementation
 ******************************************************************************/

#include "EditorLayer.h"
#include "Core/Log.h"
#include "Core/Application.h"
#include <imgui.h>
#include <glad/gl.h>
#include <cmath>
#include "Rendering/Renderer.h"
#include "Rendering/Shader.h"
#include "Rendering/Pass/GeometryPass.h"
#include "Rendering/Pass/SkyPass.h"
#include "Rendering/Pass/WaterPass.h"
#include "Rendering/Pass/TerrainPass.h"
#include "Rendering/Pass/GrassPass.h"
#include "Rendering/Pass/PostProcessPass.h"
#include "Rendering/Pass/SkeletalAnimationPass.h"
#include "Rendering/Pass/ParticlePass.h"
#include "ECS/Components.h"
#include "Resource/Mesh.h"
#include "Resource/AssetDatabase.h"
#include "Resource/AssimpMeshLoader.h"
#include "Scripting/ScriptSystem.h"

namespace MyEngine {

EditorLayer::EditorLayer()
    : Layer("EditorLayer")
{
    ENGINE_INFO("EditorLayer constructed");
}

EditorLayer::~EditorLayer() {
    ENGINE_INFO("EditorLayer destroyed");
}

void EditorLayer::OnAttach() {
    ENGINE_INFO("EditorLayer attached to application");
    
    // Initialize editor camera for good terrain/water view
    m_EditorCamera.SetViewportSize(1280.0f, 720.0f);
    m_EditorCamera.SetDistance(80.0f);     // Further back to see terrain
    m_EditorCamera.SetPitch(35.0f);        // Look down from higher angle
    m_EditorCamera.SetYaw(45.0f);          // 45 degree rotation for better view
    
    // Initialize panels
    InitializePanels();
    
    // Create viewport shader
    const char* vertexSrc = R"(
        #version 330 core
        layout (location = 0) in vec3 a_Position;
        layout (location = 1) in vec3 a_Normal;
        
        uniform mat4 u_ViewProjection;
        uniform mat4 u_Transform;
        
        out vec3 v_Normal;
        out vec3 v_FragPos;
        
        void main() {
            v_FragPos = vec3(u_Transform * vec4(a_Position, 1.0));
            v_Normal = mat3(transpose(inverse(u_Transform))) * a_Normal;
            gl_Position = u_ViewProjection * vec4(v_FragPos, 1.0);
        }
    )";
    
    const char* fragmentSrc = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec3 v_Normal;
        in vec3 v_FragPos;
        
        uniform vec3 u_LightPos;
        uniform vec3 u_LightColor;
        uniform vec3 u_ObjectColor;
        uniform vec3 u_CameraPos;
        
        void main() {
            // Ambient
            float ambientStrength = 0.4;  // Increased ambient for better visibility
            vec3 ambient = ambientStrength * u_LightColor;
            
            // Diffuse
            vec3 norm = normalize(v_Normal);
            vec3 lightDir = normalize(u_LightPos - v_FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * u_LightColor;
            
            // Specular
            float specularStrength = 0.3;
            vec3 viewDir = normalize(u_CameraPos - v_FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
            vec3 specular = specularStrength * spec * u_LightColor;
            
            vec3 result = (ambient + diffuse + specular) * u_ObjectColor;
            FragColor = vec4(result, 1.0);
        }
    )";
    
    m_ViewportShader.reset(Shader::Create("ViewportShader", vertexSrc, fragmentSrc));
    ENGINE_INFO("Viewport shader created successfully");
    
    // Initialize Pass System
    auto skyPassPtr = std::make_unique<SkyPass>();
    skyPassPtr->SetPriority(10);  // Render sky first
    skyPassPtr->SetEnabled(true);  // Enabled by default for immediate visibility
    
    auto waterPassPtr = std::make_unique<WaterPass>();
    waterPassPtr->SetPriority(20);  // Render water after sky
    waterPassPtr->SetEnabled(true);  // Enabled by default for immediate visibility
    
    auto terrainPassPtr = std::make_unique<TerrainPass>();
    terrainPassPtr->SetPriority(30);  // Render terrain after water
    terrainPassPtr->SetEnabled(true);  // Enabled by default for immediate visibility
    
    auto grassPassPtr = std::make_unique<GrassPass>();
    grassPassPtr->SetPriority(40);  // Render grass after terrain
    grassPassPtr->SetEnabled(true);  // Enabled by default
    
    auto skeletalAnimPassPtr = std::make_unique<SkeletalAnimationPass>();
    skeletalAnimPassPtr->SetPriority(50);  // Render animated model after grass
    skeletalAnimPassPtr->SetEnabled(true);  // Enabled by default
    
    auto geometryPassPtr = std::make_unique<GeometryPass>();
    geometryPassPtr->SetShader(m_ViewportShader.get());
    geometryPassPtr->SetPriority(100);  // Render geometry after environment
    
    auto postProcessPassPtr = std::make_unique<PostProcessPass>();
    postProcessPassPtr->SetPriority(1000);  // Render last
    postProcessPassPtr->SetEnabled(false);  // Disabled by default
    
    // Particle passes (VFX)
    auto fireParticlePassPtr = std::make_unique<ParticlePass>(ParticlePreset::Fire);
    fireParticlePassPtr->SetPriority(200);  // Render after geometry
    fireParticlePassPtr->SetEnabled(true);
    
    auto smokeParticlePassPtr = std::make_unique<ParticlePass>(ParticlePreset::Smoke);
    smokeParticlePassPtr->SetPriority(201);
    smokeParticlePassPtr->SetEnabled(true);
    
    auto explosionParticlePassPtr = std::make_unique<ParticlePass>(ParticlePreset::Explosion);
    explosionParticlePassPtr->SetPriority(202);
    explosionParticlePassPtr->SetEnabled(true);
    
    m_PassManager.AddPass(std::move(skyPassPtr));
    m_PassManager.AddPass(std::move(waterPassPtr));
    m_PassManager.AddPass(std::move(terrainPassPtr));
    m_PassManager.AddPass(std::move(grassPassPtr));
    m_PassManager.AddPass(std::move(skeletalAnimPassPtr));
    m_PassManager.AddPass(std::move(geometryPassPtr));
    m_PassManager.AddPass(std::move(fireParticlePassPtr));
    m_PassManager.AddPass(std::move(smokeParticlePassPtr));
    m_PassManager.AddPass(std::move(explosionParticlePassPtr));
    m_PassManager.AddPass(std::move(postProcessPassPtr));
    
    // Initialize passes with backend
    m_PassManager.OnCreate(Renderer::GetBackend());
    
    auto passCount = m_PassManager.GetAllPasses().size();
    ENGINE_INFO("Pass system initialized: {} passes", passCount);
    if (passCount == 0) {
        ENGINE_WARN("No passes registered in PassManager!");
    }
}

void EditorLayer::OnDetach() {
    ENGINE_INFO("EditorLayer detached from application");
    m_Panels.clear();
}

void EditorLayer::OnUpdate(float deltaTime) {
    // Update Lua scripts (only in play mode or if always updating)
    if (m_ScriptSystem && m_IsPlaying) {
        m_ScriptSystem->Update(deltaTime);
    }
    
    // Get viewport panel to check hover state
    ViewportPanel* viewportPanel = nullptr;
    if (m_Panels.size() >= 3) {
        viewportPanel = static_cast<ViewportPanel*>(m_Panels[2].get());
        m_ViewportHovered = viewportPanel->IsHovered();
        m_ViewportFocused = viewportPanel->IsFocused();
    }
    
    // Handle camera input when viewport is hovered
    ImGuiIO& io = ImGui::GetIO();
    
    // Camera control when viewport is hovered and not blocking ImGui
    if (m_ViewportHovered && !ImGui::IsAnyItemActive()) {
        Vec2 mousePos(io.MousePos.x, io.MousePos.y);
        Vec2 mouseDelta = mousePos - m_LastMousePosition;
        
        // Right mouse button: Rotate camera
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            m_EditorCamera.MouseRotate(mouseDelta);
            m_EditorCamera.UpdateView();
        }
        
        // Middle mouse button: Pan camera
        if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
            m_EditorCamera.MousePan(mouseDelta);
            m_EditorCamera.UpdateView();
        }
        
        // Mouse wheel: Zoom
        if (io.MouseWheel != 0.0f) {
            m_EditorCamera.MouseZoom(io.MouseWheel);
            m_EditorCamera.UpdateView();
        }
        
        m_LastMousePosition = mousePos;
    } else {
        // Update last position even when not hovering to avoid jumps
        m_LastMousePosition = Vec2(io.MousePos.x, io.MousePos.y);
    }
    
    // Update editor camera
    m_EditorCamera.OnUpdate(deltaTime);
    
    // Keyboard camera movement (WASD+QE)
    m_EditorCamera.KeyboardMove(deltaTime);
    
    // Render scene to viewport framebuffer
    if (viewportPanel) {
        RenderViewport(viewportPanel);
    }
    
    // Update all panels
    UpdatePanels(deltaTime);
}

void EditorLayer::OnUIRender() {
    // Show demo window (controlled by menu)
    if (m_ShowDemoWindow) {
        ImGui::ShowDemoWindow(&m_ShowDemoWindow);
    }
    
    // 渲染菜单栏 (with integrated Play/Stop controls)
    RenderMenuBar();
    
    // Sync selection from Scene Hierarchy to Properties
    if (m_Panels.size() >= 2) {
        auto sceneHierarchy = std::static_pointer_cast<SceneHierarchyPanel>(m_Panels[0]);
        auto properties = std::static_pointer_cast<PropertiesPanel>(m_Panels[1]);
        
        // Get current selection from hierarchy
        Entity selectedEntity = sceneHierarchy->GetSelectedEntity();
        
        // Update properties panel with selected entity
        properties->SetSelectedEntity(selectedEntity);
    }
    
    // 渲染所有面板
    for (auto& panel : m_Panels) {
        if (panel->IsOpen()) {
            panel->OnUIRender();
        }
    }
    
    // Render scene camera mode overlay (must be after panels)
    if (m_UseSceneCamera) {
        // Find viewport panel to get bounds
        ViewportPanel* viewportPanel = nullptr;
        if (m_Panels.size() >= 3) {
            viewportPanel = dynamic_cast<ViewportPanel*>(m_Panels[2].get());
        }
        
        if (viewportPanel) {
            ImGui::SetNextWindowPos(ImVec2(viewportPanel->GetViewportBounds()[0].x + 10, 
                                           viewportPanel->GetViewportBounds()[0].y + 10));
            ImGui::SetNextWindowBgAlpha(0.5f);
            ImGui::Begin("Scene Camera Overlay", nullptr, 
                         ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | 
                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | 
                         ImGuiWindowFlags_NoNav);
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "SCENE CAMERA MODE");
            ImGui::Separator();
            ImGui::Text("Gizmo controls disabled");
            ImGui::Text("Edit MainCamera position in Properties panel");
            ImGui::Text("Uncheck 'View > Use Scene Camera' to use Editor Camera");
            ImGui::End();
        }
    }
}

void EditorLayer::OnEvent(Event& event) {
    // 编辑器摄像机处理事件
    m_EditorCamera.OnEvent(event);
    
    // 面板处理事件
    for (auto& panel : m_Panels) {
        if (panel->IsOpen()) {
            panel->OnEvent(event);
        }
    }
}

void EditorLayer::SetActiveScene(Registry* registry) {
    m_ActiveRegistry = registry;
    ENGINE_INFO("Active scene set");
    
    // Update SceneHierarchyPanel context
    ENGINE_INFO("Panel count: {}", m_Panels.size());
    if (m_Panels.size() >= 1) {
        auto sceneHierarchy = std::static_pointer_cast<SceneHierarchyPanel>(m_Panels[0]);
        sceneHierarchy->SetContext(m_ActiveRegistry);
        ENGINE_INFO("Updated SceneHierarchyPanel context");
    } else {
        ENGINE_WARN("Not enough panels to update SceneHierarchyPanel");
    }
    
    // Auto-create default pass entities at startup
    // TerrainPass - always visible by default
    auto* terrainPass = m_PassManager.GetPass("TerrainPass");
    if (terrainPass && m_ActiveRegistry) {
        CreatePassEntity("TerrainPass", terrainPass, "TerrainPass");
        ENGINE_INFO("Auto-created TerrainPass entity at startup");
    }
    
    // WaterPass - always visible by default
    auto* waterPass = m_PassManager.GetPass("WaterPass");
    if (waterPass && m_ActiveRegistry) {
        CreatePassEntity("WaterPass", waterPass, "WaterPass");
        ENGINE_INFO("Auto-created WaterPass entity at startup");
    }
    
    // GrassPass - always visible by default
    auto* grassPass = m_PassManager.GetPass("GrassPass");
    if (grassPass && m_ActiveRegistry) {
        CreatePassEntity("GrassPass", grassPass, "GrassPass");
        ENGINE_INFO("Auto-created GrassPass entity at startup");
    }
    
    // SkeletalAnimationPass - always visible by default
    auto* skeletalAnimPass = m_PassManager.GetPass("SkeletalAnimationPass");
    if (skeletalAnimPass && m_ActiveRegistry) {
        CreatePassEntity("SkeletalAnimationPass", skeletalAnimPass, "SkeletalAnimationPass");
        ENGINE_INFO("Auto-created SkeletalAnimationPass entity at startup");
    }
    
    // Auto-create 3 particle effect entities (Fire, Smoke, Explosion)
    auto* firePass = m_PassManager.GetPass("ParticlePass_Fire");
    if (firePass && m_ActiveRegistry) {
        CreatePassEntity("FireEffect", firePass, "ParticlePass_Fire");
        ENGINE_INFO("Auto-created FireEffect entity at startup");
    }
    
    auto* smokePass = m_PassManager.GetPass("ParticlePass_Smoke");
    if (smokePass && m_ActiveRegistry) {
        CreatePassEntity("SmokeEffect", smokePass, "ParticlePass_Smoke");
        ENGINE_INFO("Auto-created SmokeEffect entity at startup");
    }
    
    auto* explosionPass = m_PassManager.GetPass("ParticlePass_Explosion");
    if (explosionPass && m_ActiveRegistry) {
        CreatePassEntity("ExplosionEffect", explosionPass, "ParticlePass_Explosion");
        ENGINE_INFO("Auto-created ExplosionEffect entity at startup");
    }
}

void EditorLayer::InitializePanels() {
    ENGINE_INFO("Initialize editor panels");
    
    // Create panels (temporarily not adding to list, waiting for ImGui integration)
    auto sceneHierarchyPanel = std::make_shared<SceneHierarchyPanel>(m_ActiveRegistry);
    auto propertiesPanel = std::make_shared<PropertiesPanel>();
    auto viewportPanel = std::make_shared<ViewportPanel>(&m_EditorCamera);
    auto assetBrowserPanel = std::make_shared<AssetBrowserPanel>();
    
    // Set up drag-and-drop callback for viewport
    viewportPanel->SetAssetDropCallback([this](const std::string& assetGUID, const Vec2& dropPos) {
        OnAssetDropped(assetGUID, dropPos);
    });
    
    // Add to panel list
    m_Panels.push_back(sceneHierarchyPanel);
    m_Panels.push_back(propertiesPanel);
    m_Panels.push_back(viewportPanel);
    m_Panels.push_back(assetBrowserPanel);
    
    ENGINE_INFO("Created {} editor panels", m_Panels.size());
}

void EditorLayer::UpdatePanels(float deltaTime) {
    for (auto& panel : m_Panels) {
        if (panel->IsOpen()) {
            panel->OnUpdate(deltaTime);
        }
    }
}

void EditorLayer::RenderMenuBar() {
    // Simple test menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                // Request window close
                ENGINE_INFO("Exit requested from menu");
                // TODO: Need to implement proper application exit
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            bool sceneHierarchyOpen = m_Panels[0]->IsOpen();
            bool propertiesOpen = m_Panels[1]->IsOpen();
            bool viewportOpen = m_Panels[2]->IsOpen();
            
            if (ImGui::MenuItem("Scene Hierarchy", nullptr, &sceneHierarchyOpen)) {
                m_Panels[0]->SetOpen(sceneHierarchyOpen);
            }
            if (ImGui::MenuItem("Properties", nullptr, &propertiesOpen)) {
                m_Panels[1]->SetOpen(propertiesOpen);
            }
            if (ImGui::MenuItem("Viewport", nullptr, &viewportOpen)) {
                m_Panels[2]->SetOpen(viewportOpen);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Use Scene Camera", nullptr, &m_UseSceneCamera)) {
                if (m_UseSceneCamera) {
                    ENGINE_INFO("Scene Camera mode enabled. Adjust MainCamera position in Properties panel.");
                } else {
                    ENGINE_INFO("Editor Camera mode enabled. Use mouse to control camera.");
                }
            }
            if (m_UseSceneCamera) {
                ImGui::TextDisabled("(Gizmo disabled - edit MainCamera in Properties)");
            }
            ImGui::MenuItem("ImGui Demo", nullptr, &m_ShowDemoWindow);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Create")) {
            ImGui::Text("Rendering Passes:");
            ImGui::Separator();
            
            if (ImGui::MenuItem("Water Pass")) {
                auto* waterPass = m_PassManager.GetPass("WaterPass");
                if (waterPass && m_ActiveRegistry) {
                    CreatePassEntity("WaterPass", waterPass, "WaterPass");
                    ENGINE_INFO("Created WaterPass entity from menu");
                }
            }
            
            if (ImGui::MenuItem("Terrain Pass")) {
                auto* terrainPass = m_PassManager.GetPass("TerrainPass");
                if (terrainPass && m_ActiveRegistry) {
                    CreatePassEntity("TerrainPass", terrainPass, "TerrainPass");
                    ENGINE_INFO("Created TerrainPass entity from menu");
                }
            }
            
            if (ImGui::MenuItem("Sky Pass")) {
                auto* skyPass = m_PassManager.GetPass("SkyPass");
                if (skyPass && m_ActiveRegistry) {
                    CreatePassEntity("SkyPass", skyPass, "SkyPass");
                    ENGINE_INFO("Created SkyPass entity from menu");
                }
            }
            
            if (ImGui::MenuItem("PostProcess Pass")) {
                auto* postProcessPass = m_PassManager.GetPass("PostProcessPass");
                if (postProcessPass && m_ActiveRegistry) {
                    CreatePassEntity("PostProcessPass", postProcessPass, "PostProcessPass");
                    ENGINE_INFO("Created PostProcessPass entity from menu");
                }
            }
            
            ImGui::Separator();
            ImGui::Text("Particle Effects");
            
            if (ImGui::MenuItem("Fire Particle")) {
                auto* firePass = m_PassManager.GetPass("ParticlePass_Fire");
                if (firePass && m_ActiveRegistry) {
                    CreatePassEntity("FireEffect", firePass, "ParticlePass_Fire");
                    ENGINE_INFO("Created Fire particle effect from menu");
                }
            }
            
            if (ImGui::MenuItem("Smoke Particle")) {
                auto* smokePass = m_PassManager.GetPass("ParticlePass_Smoke");
                if (smokePass && m_ActiveRegistry) {
                    CreatePassEntity("SmokeEffect", smokePass, "ParticlePass_Smoke");
                    ENGINE_INFO("Created Smoke particle effect from menu");
                }
            }
            
            if (ImGui::MenuItem("Explosion Particle")) {
                auto* explosionPass = m_PassManager.GetPass("ParticlePass_Explosion");
                if (explosionPass && m_ActiveRegistry) {
                    CreatePassEntity("ExplosionEffect", explosionPass, "ParticlePass_Explosion");
                    ENGINE_INFO("Created Explosion particle effect from menu");
                }
            }
            
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Rendering")) {
            ImGui::MenuItem("Use Pass System", nullptr, &m_UsePassSystem);
            ImGui::Separator();
            
            // Pass controls
            auto passes = m_PassManager.GetAllPasses();
            for (auto* pass : passes) {
                bool enabled = pass->IsEnabled();
                if (ImGui::MenuItem(pass->GetName(), nullptr, &enabled)) {
                    pass->SetEnabled(enabled);
                }
            }
            
            ImGui::Separator();
            ImGui::Text("Pass Count: %zu", passes.size());
            ImGui::EndMenu();
        }
        
        // Add spacing to push Play/Stop controls to the right
        float menuBarWidth = ImGui::GetWindowWidth();
        float playControlsWidth = 150.0f;
        float infoTextWidth = 250.0f;
        ImGui::SameLine(menuBarWidth - playControlsWidth - infoTextWidth);
        
        // Play/Stop controls
        if (m_IsPlaying) {
            // Stop button (red)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
            
            if (ImGui::Button("■ Stop", ImVec2(70, 0))) {
                ExitPlayMode();
            }
            
            ImGui::PopStyleColor(3);
            
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Playing");
        } else {
            // Play button (green)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
            
            if (ImGui::Button("▶ Play", ImVec2(70, 0))) {
                EnterPlayMode();
            }
            
            ImGui::PopStyleColor(3);
        }
        
        // Info text
        ImGui::SameLine();
        ImGui::Text(" | MyEngine Editor v0.1 | FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::EndMainMenuBar();
    }
}

void EditorLayer::RenderToolbar() {
    // Toolbar with play/stop buttons
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    ImGui::Begin("Toolbar", nullptr, 
        ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoScrollbar | 
        ImGuiWindowFlags_NoScrollWithMouse);
    
    // Left side - title
    ImGui::Text("MyEngine Editor");
    
    ImGui::SameLine(ImGui::GetWindowWidth() * 0.5f - 50);
    
    // Center - Play/Stop controls
    if (m_IsPlaying) {
        // Stop button (red)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
        
        if (ImGui::Button("■ Stop", ImVec2(80, 0))) {
            ExitPlayMode();
        }
        
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Playing");
    } else {
        // Play button (green)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
        
        if (ImGui::Button("▶ Play", ImVec2(80, 0))) {
            EnterPlayMode();
        }
        
        ImGui::PopStyleColor(3);
    }
    
    ImGui::End();
    ImGui::PopStyleVar();
}

void EditorLayer::RenderViewport(ViewportPanel* viewport) {
    if (!viewport || !m_ActiveRegistry) return;
    
    // Get viewport framebuffer
    uint32_t framebufferID = viewport->GetFramebufferID();
    if (framebufferID == 0) return;  // Framebuffer not created yet
    
    // Bind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
    
    // Clear viewport
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);  // Blue-gray background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Setup camera matrices
    Mat4 viewMatrix;
    Mat4 projectionMatrix;
    Vec3 cameraPos;
    
    if (m_UseSceneCamera && m_ActiveRegistry) {
        // Use scene camera (MainCamera entity)
        auto cameraEntities = m_ActiveRegistry->GetEntitiesWith<TagComponent, TransformComponent>();
        bool foundCamera = false;
        for (auto entityID : cameraEntities) {
            Entity entity(entityID, m_ActiveRegistry);
            auto& tag = entity.GetComponent<TagComponent>();
            if (tag.Tag == "MainCamera") {
                auto& transform = entity.GetComponent<TransformComponent>();
                
                // Ensure transform is up-to-date
                if (transform.localVersion != transform.worldVersion) {
                    transform.UpdateLocalMatrix();
                }
                
                cameraPos = transform.localPosition;
                
                // Apply rotation to camera basis vectors
                // Quaternion rotation: v' = q * v * q^(-1)
                // For unit quaternions: q^(-1) = conjugate(q)
                Quat q = transform.localRotation;
                
                // Helper lambda to rotate a vector by quaternion
                auto rotateVec = [](const Quat& q, const Vec3& v) -> Vec3 {
                    // q * v * q^(-1) where v is treated as quaternion (0, v)
                    float qx = q.x, qy = q.y, qz = q.z, qw = q.w;
                    float vx = v.x, vy = v.y, vz = v.z;
                    
                    // First: q * v
                    float tx = qw * vx + qy * vz - qz * vy;
                    float ty = qw * vy + qz * vx - qx * vz;
                    float tz = qw * vz + qx * vy - qy * vx;
                    float tw = -qx * vx - qy * vy - qz * vz;
                    
                    // Second: result * q^(-1) = result * conjugate(q)
                    float rx = tw * (-qx) + tx * qw + ty * (-qz) - tz * (-qy);
                    float ry = tw * (-qy) + ty * qw + tz * (-qx) - tx * (-qz);
                    float rz = tw * (-qz) + tz * qw + tx * (-qy) - ty * (-qx);
                    
                    return Vec3(rx, ry, rz);
                };
                
                // Camera default orientation: forward=-Z, up=+Y, right=+X
                Vec3 forward = rotateVec(q, Vec3(0, 0, -1));
                Vec3 up = rotateVec(q, Vec3(0, 1, 0));
                Vec3 right = rotateVec(q, Vec3(1, 0, 0));
                
                // Build view matrix: inverse of camera transform
                viewMatrix.Identity();
                // Rotation part (transpose of camera basis)
                viewMatrix.m[0] = right.x;   viewMatrix.m[4] = right.y;   viewMatrix.m[8] = right.z;
                viewMatrix.m[1] = up.x;      viewMatrix.m[5] = up.y;      viewMatrix.m[9] = up.z;
                viewMatrix.m[2] = -forward.x; viewMatrix.m[6] = -forward.y; viewMatrix.m[10] = -forward.z;
                // Translation part
                viewMatrix.m[12] = -Vec3::Dot(right, cameraPos);
                viewMatrix.m[13] = -Vec3::Dot(up, cameraPos);
                viewMatrix.m[14] = Vec3::Dot(forward, cameraPos);
                viewMatrix.m[15] = 1.0f;
                
                // Use same projection as editor camera
                projectionMatrix = m_EditorCamera.GetProjectionMatrix();
                foundCamera = true;
                break;
            }
        }
        
        if (!foundCamera) {
            // Fallback to editor camera if MainCamera not found
            viewMatrix = m_EditorCamera.GetViewMatrix();
            projectionMatrix = m_EditorCamera.GetProjectionMatrix();
            cameraPos = m_EditorCamera.GetPosition();
        }
    } else {
        // Use editor camera
        viewMatrix = m_EditorCamera.GetViewMatrix();
        projectionMatrix = m_EditorCamera.GetProjectionMatrix();
        cameraPos = m_EditorCamera.GetPosition();
    }
    
    // Create scene view for passes
    SceneView sceneView(viewMatrix, projectionMatrix, cameraPos);
    
    if (m_UsePassSystem) {
        // ===== NEW: Pass-based rendering =====
        m_PassManager.Execute(sceneView, m_ActiveRegistry);
    } else {
        // ===== OLD: Legacy rendering (for comparison) =====
        if (!m_ViewportShader) return;
        
        // Enable depth testing
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDisable(GL_CULL_FACE);
        
        // Begin scene with editor camera
        Renderer::BeginScene(viewMatrix, projectionMatrix);
        
        // Bind shader
        m_ViewportShader->Bind();
        
        // Set view-projection matrix
        Mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
        m_ViewportShader->SetMat4("u_ViewProjection", viewProjectionMatrix);
        m_ViewportShader->SetFloat3("u_CameraPos", cameraPos);
        
        // Render all entities with MeshFilterComponent
        auto view = m_ActiveRegistry->GetEntitiesWith<TransformComponent, MeshFilterComponent>();
        for (EntityID entityID : view) {
            Entity entity(entityID, m_ActiveRegistry);
            
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& meshFilter = entity.GetComponent<MeshFilterComponent>();
            
            // Build model matrix from transform
            Mat4 translationMat = Mat4::Translation(transform.localPosition);
            Mat4 rotationMat = Mat4(); // Identity for now
            Mat4 scaleMat = Mat4::Scale(transform.localScale);
            Mat4 modelMatrix = translationMat * rotationMat * scaleMat;
            
            // Set model matrix uniform
            m_ViewportShader->SetMat4("u_Transform", modelMatrix);
            
            // Render the mesh
            if (meshFilter.mesh) {
                Renderer::DrawMesh(*meshFilter.mesh, modelMatrix);
            }
        }
        
        // End scene
        Renderer::EndScene();
    }
    
    // Check for OpenGL errors
    GLenum err = glGetError();
    static bool errorLogged = false;
    if (err != GL_NO_ERROR && !errorLogged) {
        ENGINE_ERROR("OpenGL error in viewport rendering: {}", err);
        errorLogged = true;
    }
    
    // Unbind framebuffer (return to default)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void EditorLayer::CreatePassEntity(const std::string& name, RenderPass* pass, const std::string& passType) {
    if (!m_ActiveRegistry || !pass) return;
    
    // Create entity for this pass
    EntityID passEntityID = m_ActiveRegistry->CreateEntity();
    Entity passEntity(passEntityID, m_ActiveRegistry);
    
    // Add components
    passEntity.AddComponent<TagComponent>(name);
    passEntity.AddComponent<TransformComponent>();  // For hierarchy compatibility
    passEntity.AddComponent<PassComponent>(pass, passType);
    
    ENGINE_INFO("Created Pass entity '{}' with ID {}", name, passEntityID);
}

void EditorLayer::OnAssetDropped(const std::string& assetGUID, const Vec2& dropPosition) {
    ENGINE_INFO("Processing asset drop: GUID={}, Position=({}, {})", assetGUID, dropPosition.x, dropPosition.y);
    
    // Get asset metadata from database
    auto& assetDB = AssetDatabase::GetInstance();
    const AssetMetadata* metadata = assetDB.GetMetadata(assetGUID);
    
    if (!metadata) {
        ENGINE_WARN("Asset metadata not found for GUID: {}", assetGUID);
        return;
    }
    
    ENGINE_INFO("Instantiating asset: {} (Type: {})", metadata->name, AssetTypeToString(metadata->type));
    
    // Convert screen position to world position
    // Get viewport panel to retrieve viewport size
    Vec2 viewportSize(1280.0f, 720.0f);  // Default size
    if (m_Panels.size() >= 3) {
        auto viewportPanel = std::static_pointer_cast<ViewportPanel>(m_Panels[2]);
        viewportSize = viewportPanel->GetViewportSize();
    }
    
    Vec3 worldPosition = ScreenToWorld(dropPosition, viewportSize);
    ENGINE_INFO("Converted screen position ({}, {}) to world position ({}, {}, {})", 
        dropPosition.x, dropPosition.y, worldPosition.x, worldPosition.y, worldPosition.z);
    
    // Create entity based on asset type
    EntityID newEntityID = 0;
    
    switch (metadata->type) {
        case AssetType::Model:
        case AssetType::Mesh: {
            // Create a mesh entity
            newEntityID = m_ActiveRegistry->CreateEntity();
            Entity entity(newEntityID, m_ActiveRegistry);
            
            // Add components
            entity.AddComponent<TagComponent>(metadata->name);
            
            // Add transform component with calculated world position
            auto& transform = entity.AddComponent<TransformComponent>();
            transform.localPosition = worldPosition;
            transform.localScale = Vec3(1.0f, 1.0f, 1.0f);
            
            // Load the actual mesh from file
            try {
                auto loadedMesh = AssimpMeshLoader::Load(metadata->path);
                if (loadedMesh) {
                    // Add mesh filter component with loaded mesh
                    auto& meshFilter = entity.AddComponent<MeshFilterComponent>();
                    meshFilter.mesh = loadedMesh;
                    ENGINE_INFO("Loaded mesh from '{}': {} vertices, {} indices", 
                        metadata->path, loadedMesh->GetVertices().size(), loadedMesh->GetIndices().size());
                } else {
                    ENGINE_WARN("Failed to load mesh from '{}'", metadata->path);
                }
            } catch (const std::exception& e) {
                ENGINE_ERROR("Exception loading mesh '{}': {}", metadata->path, e.what());
            }
            
            // Add to scene graph
            auto sceneNode = m_SceneGraph.CreateNode(newEntityID, metadata->name);
            m_SceneGraph.AddRootNode(sceneNode);
            
            ENGINE_INFO("Created mesh entity '{}' with ID {}", metadata->name, newEntityID);
            break;
        }
        
        case AssetType::Texture: {
            // Create a sprite/plane entity with texture
            newEntityID = m_ActiveRegistry->CreateEntity();
            Entity entity(newEntityID, m_ActiveRegistry);
            
            entity.AddComponent<TagComponent>(metadata->name);
            
            auto& transform = entity.AddComponent<TransformComponent>();
            transform.localPosition = Vec3(0.0f, 0.0f, 0.0f);
            transform.localScale = Vec3(1.0f, 1.0f, 1.0f);
            
            // Add to scene graph
            auto sceneNode = m_SceneGraph.CreateNode(newEntityID, metadata->name);
            m_SceneGraph.AddRootNode(sceneNode);
            
            ENGINE_INFO("Created texture entity '{}' with ID {}", metadata->name, newEntityID);
            break;
        }
        
        case AssetType::Prefab: {
            // Instantiate prefab
            ENGINE_INFO("Prefab instantiation not yet implemented");
            break;
        }
        
        default:
            ENGINE_WARN("Asset type '{}' cannot be instantiated in scene", AssetTypeToString(metadata->type));
            return;
    }
    
    // Select the newly created entity
    if (newEntityID != 0) {
        m_SelectedEntity = newEntityID;
        
        // Update scene hierarchy panel
        if (m_Panels.size() >= 1) {
            auto sceneHierarchy = std::static_pointer_cast<SceneHierarchyPanel>(m_Panels[0]);
            sceneHierarchy->SetSelectedEntity(Entity(newEntityID, m_ActiveRegistry));
        }
    }
}

Vec3 EditorLayer::ScreenToWorld(const Vec2& screenPos, const Vec2& viewportSize) const {
    // Convert screen coordinates to normalized device coordinates (NDC)
    // Screen space: origin at top-left, Y increases downward
    // NDC: origin at center, X in [-1, 1], Y in [-1, 1], Z in [-1, 1]
    
    float x = (2.0f * screenPos.x) / viewportSize.x - 1.0f;
    float y = 1.0f - (2.0f * screenPos.y) / viewportSize.y;  // Flip Y axis
    
    // Get camera matrices
    Mat4 viewMatrix = m_EditorCamera.GetViewMatrix();
    Mat4 projMatrix = m_EditorCamera.GetProjectionMatrix();
    
    // Calculate inverse matrices for unprojection
    Mat4 viewProjMatrix = projMatrix * viewMatrix;
    Mat4 invViewProjMatrix = viewProjMatrix.Inverted();
    
    // Create two points in NDC space: near plane (z = -1) and far plane (z = 1)
    Vec4 nearPoint(x, y, -1.0f, 1.0f);
    Vec4 farPoint(x, y, 1.0f, 1.0f);
    
    // Transform to world space
    Vec4 nearWorld = invViewProjMatrix * nearPoint;
    Vec4 farWorld = invViewProjMatrix * farPoint;
    
    // Perform perspective divide
    if (nearWorld.w != 0.0f) {
        nearWorld.x /= nearWorld.w;
        nearWorld.y /= nearWorld.w;
        nearWorld.z /= nearWorld.w;
    }
    if (farWorld.w != 0.0f) {
        farWorld.x /= farWorld.w;
        farWorld.y /= farWorld.w;
        farWorld.z /= farWorld.w;
    }
    
    // Create ray from near to far
    Vec3 rayOrigin(nearWorld.x, nearWorld.y, nearWorld.z);
    Vec3 rayEnd(farWorld.x, farWorld.y, farWorld.z);
    Vec3 rayDirection = Vec3(
        rayEnd.x - rayOrigin.x,
        rayEnd.y - rayOrigin.y,
        rayEnd.z - rayOrigin.z
    );
    
    // Normalize ray direction
    float length = std::sqrt(rayDirection.x * rayDirection.x + 
                             rayDirection.y * rayDirection.y + 
                             rayDirection.z * rayDirection.z);
    if (length > 0.0001f) {
        rayDirection.x /= length;
        rayDirection.y /= length;
        rayDirection.z /= length;
    }
    
    // Intersect ray with a horizontal plane at Y = 0 (ground plane)
    // Ray equation: P = rayOrigin + t * rayDirection
    // Plane equation: Y = 0
    // Solve for t: rayOrigin.y + t * rayDirection.y = 0
    
    Vec3 worldPosition;
    if (std::abs(rayDirection.y) > 0.0001f) {
        float t = -rayOrigin.y / rayDirection.y;
        worldPosition = Vec3(
            rayOrigin.x + t * rayDirection.x,
            0.0f,  // On the ground plane
            rayOrigin.z + t * rayDirection.z
        );
    } else {
        // Ray is parallel to ground, use a fixed distance from camera
        float distance = 20.0f;
        worldPosition = Vec3(
            rayOrigin.x + distance * rayDirection.x,
            rayOrigin.y + distance * rayDirection.y,
            rayOrigin.z + distance * rayDirection.z
        );
    }
    
    return worldPosition;
}

void EditorLayer::EnterPlayMode() {
    ENGINE_INFO("Entering Play Mode");
    m_IsPlaying = true;
    
    // Switch application to Game mode
    Application::Get().SetMode(EngineMode::Game);
    
    // Save editor camera state
    // TODO: Save scene state for restoration on exit
    
    // Disable editor-specific features
    // TODO: Disable gizmos, selection, etc.
}

void EditorLayer::ExitPlayMode() {
    ENGINE_INFO("Exiting Play Mode");
    m_IsPlaying = false;
    
    // Switch application back to Editor mode
    Application::Get().SetMode(EngineMode::Editor);
    
    // Restore editor camera state
    // TODO: Restore scene state
    
    // Re-enable editor-specific features
    // TODO: Re-enable gizmos, selection, etc.
}

} // namespace MyEngine
