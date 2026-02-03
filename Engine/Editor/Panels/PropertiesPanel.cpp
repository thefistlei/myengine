/******************************************************************************
 * File: PropertiesPanel.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Properties panel implementation
 ******************************************************************************/

#include "PropertiesPanel.h"
#include "ECS/Entity.h"
#include "ECS/Components.h"
#include "Rendering/Pass/RenderPass.h"
#include "Rendering/Pass/WaterPass.h"
#include "Rendering/Pass/TerrainPass.h"
#include "Rendering/Pass/PostProcessPass.h"
#include "Rendering/Pass/SkyPass.h"
#include "Rendering/Pass/GeometryPass.h"
#include "Rendering/Pass/SkeletalAnimationPass.h"
#include "Rendering/Pass/ParticlePass.h"
#include "Core/Log.h"
#include <imgui.h>
#include <cmath>

namespace MyEngine {

PropertiesPanel::PropertiesPanel()
    : Panel("Properties", true)
{
}

void PropertiesPanel::OnUIRender() {
    if (!m_IsOpen) return;
    
    ImGui::Begin(m_Name.c_str(), &m_IsOpen);
    
    if (m_SelectionContext) {
        DrawComponents(m_SelectionContext);
    } else {
        ImGui::Text("No entity selected");
    }
    
    ImGui::End();
}

void PropertiesPanel::OnUpdate(float deltaTime) {
    // Can add real-time update logic here
}

void PropertiesPanel::DrawComponents(Entity entity) {
    // Draw all components
    DrawTagComponent(entity);
    DrawTransformComponent(entity);
    DrawHierarchyComponent(entity);
    DrawMeshFilterComponent(entity);
    DrawPassComponent(entity);
    
    // Add component button
    DrawAddComponentButton(entity);
}

void PropertiesPanel::DrawTagComponent(Entity entity) {
    if (!entity.HasComponent<TagComponent>()) return;
    
    auto& tag = entity.GetComponent<TagComponent>().Tag;
    
    ImGui::Separator();
    ImGui::Text("Tag Component");
    
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, tag.c_str(), sizeof(buffer) - 1);
    
    if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
        tag = std::string(buffer);
    }
}

void PropertiesPanel::DrawTransformComponent(Entity entity) {
    if (!entity.HasComponent<TransformComponent>()) return;
    
    auto& tc = entity.GetComponent<TransformComponent>();
    
    ImGui::Separator();
    if (ImGui::TreeNodeEx("Transform Component", ImGuiTreeNodeFlags_DefaultOpen)) {
        
        // Position
        float position[3] = { tc.localPosition.x, tc.localPosition.y, tc.localPosition.z };
        if (ImGui::DragFloat3("Position", position, 0.1f)) {
            tc.localPosition = Vec3(position[0], position[1], position[2]);
            tc.UpdateLocalMatrix();
            // Force version sync to ensure rendering passes see the update
            tc.worldVersion = tc.localVersion;
            ENGINE_INFO("Position updated: ({}, {}, {}), version: {}", position[0], position[1], position[2], tc.localVersion);
        }
        
        // Rotation (Euler angles in degrees)
        // Convert quaternion to Euler angles for editing
        auto quatToEuler = [](const Quat& q) -> Vec3 {
            // Convert quaternion to Euler angles (XYZ order, in radians)
            float x = q.x, y = q.y, z = q.z, w = q.w;
            
            // Roll (X-axis rotation)
            float sinr_cosp = 2.0f * (w * x + y * z);
            float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
            float roll = std::atan2(sinr_cosp, cosr_cosp);
            
            // Pitch (Y-axis rotation)
            float sinp = 2.0f * (w * y - z * x);
            float pitch;
            if (std::abs(sinp) >= 1.0f)
                pitch = std::copysign(3.14159265f / 2.0f, sinp); // use 90 degrees if out of range
            else
                pitch = std::asin(sinp);
            
            // Yaw (Z-axis rotation)
            float siny_cosp = 2.0f * (w * z + x * y);
            float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
            float yaw = std::atan2(siny_cosp, cosy_cosp);
            
            // Convert to degrees
            return Vec3(roll * 57.2958f, pitch * 57.2958f, yaw * 57.2958f);
        };
        
        auto eulerToQuat = [](const Vec3& euler) -> Quat {
            // Convert Euler angles (in degrees) to quaternion (XYZ order)
            float roll = euler.x * 0.0174533f;  // to radians
            float pitch = euler.y * 0.0174533f;
            float yaw = euler.z * 0.0174533f;
            
            float cy = std::cos(yaw * 0.5f);
            float sy = std::sin(yaw * 0.5f);
            float cp = std::cos(pitch * 0.5f);
            float sp = std::sin(pitch * 0.5f);
            float cr = std::cos(roll * 0.5f);
            float sr = std::sin(roll * 0.5f);
            
            Quat q;
            q.w = cr * cp * cy + sr * sp * sy;
            q.x = sr * cp * cy - cr * sp * sy;
            q.y = cr * sp * cy + sr * cp * sy;
            q.z = cr * cp * sy - sr * sp * cy;
            
            return q;
        };
        
        Vec3 euler = quatToEuler(tc.localRotation);
        float eulerAngles[3] = { euler.x, euler.y, euler.z };
        if (ImGui::DragFloat3("Rotation", eulerAngles, 0.5f)) {
            tc.localRotation = eulerToQuat(Vec3(eulerAngles[0], eulerAngles[1], eulerAngles[2]));
            tc.UpdateLocalMatrix();
            // Force version sync
            tc.worldVersion = tc.localVersion;
        }
        
        // Scale
        float scale[3] = { tc.localScale.x, tc.localScale.y, tc.localScale.z };
        if (ImGui::DragFloat3("Scale", scale, 0.1f, 0.001f, 1000.0f)) {
            tc.localScale = Vec3(scale[0], scale[1], scale[2]);
            tc.UpdateLocalMatrix();
            // Force version sync
            tc.worldVersion = tc.localVersion;
        }
        
        // Display version info (for debugging)
        ImGui::Separator();
        ImGui::Text("Version Info:");
        ImGui::Text("  Local: %u", tc.localVersion);
        ImGui::Text("  World: %u", tc.worldVersion);
        ImGui::Text("  Parent: %u", tc.parentVersion);
        
        ImGui::TreePop();
    }
}

void PropertiesPanel::DrawHierarchyComponent(Entity entity) {
    if (!entity.HasComponent<HierarchyComponent>()) return;
    
    auto& hc = entity.GetComponent<HierarchyComponent>();
    
    ImGui::Separator();
    if (ImGui::TreeNodeEx("Hierarchy Component", ImGuiTreeNodeFlags_DefaultOpen)) {
        
        ImGui::Text("Parent: %u", hc.parent);
        ImGui::Text("First Child: %u", hc.firstChild);
        ImGui::Text("Next Sibling: %u", hc.nextSibling);
        ImGui::Text("Depth: %u", hc.depth);
        
        ImGui::Separator();
        ImGui::Text("Status:");
        ImGui::Text("  Is Root: %s", hc.IsRoot() ? "Yes" : "No");
        ImGui::Text("  Has Children: %s", hc.HasChildren() ? "Yes" : "No");
        ImGui::Text("  Has Siblings: %s", hc.HasSiblings() ? "Yes" : "No");
        
        ImGui::TreePop();
    }
}

void PropertiesPanel::DrawMeshFilterComponent(Entity entity) {
    if (!entity.HasComponent<MeshFilterComponent>()) return;
    
    auto& mfc = entity.GetComponent<MeshFilterComponent>();
    
    ImGui::Separator();
    if (ImGui::TreeNodeEx("Mesh Filter", ImGuiTreeNodeFlags_DefaultOpen)) {
        
        // Display mesh handle
        ImGui::Text("Mesh Handle: %u", mfc.MeshHandle.IsValid() ? 1 : 0);
        
        // Future features:
        // - Drag and drop assets here
        // - Display mesh statistics
        // - Mesh preview
        
        if (ImGui::Button("Load Mesh...")) {
            ENGINE_INFO("Mesh load dialog pending implementation");
        }
        
        ImGui::TreePop();
    }
}

void PropertiesPanel::DrawAddComponentButton(Entity entity) {
    ImGui::Separator();
    
    if (ImGui::Button("Add Component", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        ImGui::OpenPopup("AddComponent");
    }
    
    if (ImGui::BeginPopup("AddComponent")) {
        
        if (!entity.HasComponent<TransformComponent>()) {
            if (ImGui::MenuItem("Transform Component")) {
                entity.AddComponent<TransformComponent>();
                ImGui::CloseCurrentPopup();
            }
        }
        
        if (!entity.HasComponent<HierarchyComponent>()) {
            if (ImGui::MenuItem("Hierarchy Component")) {
                entity.AddComponent<HierarchyComponent>();
                ImGui::CloseCurrentPopup();
            }
        }
        
        if (!entity.HasComponent<MeshFilterComponent>()) {
            if (ImGui::MenuItem("Mesh Filter")) {
                entity.AddComponent<MeshFilterComponent>();
                ImGui::CloseCurrentPopup();
            }
        }
        
        if (!entity.HasComponent<MeshRendererComponent>()) {
            if (ImGui::MenuItem("Mesh Renderer")) {
                entity.AddComponent<MeshRendererComponent>();
                ImGui::CloseCurrentPopup();
            }
        }
        
        ImGui::EndPopup();
    }
}

void PropertiesPanel::DrawPassComponent(Entity entity) {
    if (!entity.HasComponent<PassComponent>()) return;
    
    auto& passComp = entity.GetComponent<PassComponent>();
    if (!passComp.IsValid()) return;
    
    ImGui::Separator();
    if (ImGui::TreeNodeEx((passComp.passType + " Component").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
        
        // Common pass properties
        bool enabled = passComp.pass->IsEnabled();
        if (ImGui::Checkbox("Enabled", &enabled)) {
            passComp.pass->SetEnabled(enabled);
        }
        
        ImGui::Separator();
        
        // Pass-specific UI
        if (passComp.passType == "WaterPass") {
            auto* waterPass = static_cast<WaterPass*>(passComp.pass);
            waterPass->OnGUI();
        }
        else if (passComp.passType == "TerrainPass") {
            auto* terrainPass = static_cast<TerrainPass*>(passComp.pass);
            terrainPass->OnGUI();
        }
        else if (passComp.passType == "PostProcessPass") {
            auto* postProcessPass = static_cast<PostProcessPass*>(passComp.pass);
            postProcessPass->OnGUI();
        }
        else if (passComp.passType == "SkyPass") {
            auto* skyPass = static_cast<SkyPass*>(passComp.pass);
            skyPass->OnGUI();
        }
        else if (passComp.passType == "GeometryPass") {
            auto* geometryPass = static_cast<GeometryPass*>(passComp.pass);
            geometryPass->OnGUI();
        }
        else if (passComp.passType == "SkeletalAnimationPass") {
            auto* skeletalAnimPass = static_cast<SkeletalAnimationPass*>(passComp.pass);
            skeletalAnimPass->OnGUI();
        }
        else if (passComp.passType == "ParticlePass_Fire" || 
                 passComp.passType == "ParticlePass_Smoke" || 
                 passComp.passType == "ParticlePass_Explosion") {
            auto* particlePass = static_cast<ParticlePass*>(passComp.pass);
            particlePass->OnGUI();
        }
        
        ImGui::TreePop();
    }
}

} // namespace MyEngine
