/******************************************************************************
 * File: SceneHierarchyPanel.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Scene hierarchy panel implementation
 ******************************************************************************/

#include "SceneHierarchyPanel.h"
#include "Core/Log.h"
#include <imgui.h>
#include <cstring>

namespace MyEngine {

SceneHierarchyPanel::SceneHierarchyPanel(Registry* context)
    : Panel("Scene Hierarchy", true), m_Context(context)
{
}

void SceneHierarchyPanel::OnUIRender() {
    if (!m_IsOpen) return;
    
    ImGui::Begin(m_Name.c_str(), &m_IsOpen);
    
    if (m_Context) {
        // Traverse all entities
        auto view = m_Context->GetEntitiesWith<TagComponent>();
        for (auto entityID : view) {
            Entity entity{ entityID, m_Context };
            DrawEntityNode(entity);
        }
        
        // Background right-click menu
        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::MenuItem("Create Empty Entity")) {
                EntityID newEntity = m_Context->CreateEntity();
                Entity entity{ newEntity, m_Context };
                entity.AddComponent<TagComponent>("New Entity");
                entity.AddComponent<TransformComponent>();
                entity.AddComponent<HierarchyComponent>();
            }
            ImGui::EndPopup();
        }
    }
    
    // Rename dialog
    if (m_RenamingEntity) {
        ImGui::OpenPopup("Rename Entity");
        m_RenamingEntity = false;
    }
    
    if (ImGui::BeginPopupModal("Rename Entity", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter new name:");
        ImGui::InputText("##rename", m_RenameBuffer, sizeof(m_RenameBuffer));
        
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            if (m_EntityToRename && m_EntityToRename.HasComponent<TagComponent>()) {
                auto& tag = m_EntityToRename.GetComponent<TagComponent>();
                tag.Tag = m_RenameBuffer;
                ENGINE_INFO("Renamed entity to: {}", m_RenameBuffer);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    ImGui::End();
}

void SceneHierarchyPanel::OnUpdate(float deltaTime) {
    // Can add entity search, filtering functionality here
}

void SceneHierarchyPanel::DrawEntityNode(Entity entity) {
    auto& tag = entity.GetComponent<TagComponent>().Tag;
    
    ImGuiTreeNodeFlags flags = 
        ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) 
        | ImGuiTreeNodeFlags_OpenOnArrow 
        | ImGuiTreeNodeFlags_SpanAvailWidth;
    
    // If has children, add arrow
    if (entity.HasComponent<HierarchyComponent>()) {
        auto& hierarchy = entity.GetComponent<HierarchyComponent>();
        if (!hierarchy.HasChildren()) {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }
    }
    
    bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
    
    // Click to select
    if (ImGui::IsItemClicked()) {
        m_SelectionContext = entity;
    }
    
    // Right-click menu
    bool entityDeleted = false;
    
    // Generate unique ID for context menu
    std::string contextMenuID = "EntityContextMenu##" + std::to_string((uint64_t)(uint32_t)entity);
    if (ImGui::BeginPopupContextItem(contextMenuID.c_str())) {
        
        if (ImGui::MenuItem("Rename")) {
            m_RenamingEntity = true;
            m_EntityToRename = entity;
            // Copy current name to buffer
            strncpy_s(m_RenameBuffer, sizeof(m_RenameBuffer), tag.c_str(), sizeof(m_RenameBuffer) - 1);
        }
        
        if (ImGui::MenuItem("Duplicate")) {
            Entity duplicated = DuplicateEntity(entity);
            if (duplicated) {
                ENGINE_INFO("Duplicated entity: {}", tag.c_str());
                m_SelectionContext = duplicated;
            }
        }
        
        ImGui::Separator();
        
        if (ImGui::MenuItem("Create Child Entity")) {
            Entity child = CreateChildEntity(entity);
            if (child) {
                ENGINE_INFO("Created child entity under: {}", tag.c_str());
            }
        }
        
        ImGui::Separator();
        
        if (ImGui::MenuItem("Delete", "Del")) {
            entityDeleted = true;
        }
        
        ImGui::EndPopup();
    }
    
    // Draw child nodes
    if (opened) {
        if (entity.HasComponent<HierarchyComponent>()) {
            auto& hierarchy = entity.GetComponent<HierarchyComponent>();
            EntityID childID = hierarchy.firstChild;
            
            while (childID != 0) {
                Entity childEntity{ childID, m_Context };
                DrawEntityNode(childEntity);
                
                // Move to next sibling
                if (childEntity.HasComponent<HierarchyComponent>()) {
                    auto& childHierarchy = childEntity.GetComponent<HierarchyComponent>();
                    childID = childHierarchy.nextSibling;
                } else {
                    break;
                }
            }
        }
        ImGui::TreePop();
    }
    
    // Delete entity
    if (entityDeleted) {
        m_Context->DestroyEntity(entity);
        if (m_SelectionContext == entity)
            m_SelectionContext = {};
    }
}

void SceneHierarchyPanel::DrawContextMenu() {
    // Right-click menu implementation
}

void SceneHierarchyPanel::DeleteEntity(Entity entity) {
    if (!entity || !m_Context) return;
    
    // Clear selection if deleting selected entity
    if (m_SelectionContext == entity) {
        m_SelectionContext = {};
    }
    
    m_Context->DestroyEntity(entity);
    ENGINE_INFO("Deleted entity");
}

Entity SceneHierarchyPanel::DuplicateEntity(Entity source) {
    if (!source || !m_Context) return {};
    
    // Create new entity
    EntityID newEntityID = m_Context->CreateEntity();
    Entity newEntity{ newEntityID, m_Context };
    
    // Copy TagComponent
    if (source.HasComponent<TagComponent>()) {
        auto& srcTag = source.GetComponent<TagComponent>();
        auto& newTag = newEntity.AddComponent<TagComponent>(srcTag.Tag + " (Copy)");
    }
    
    // Copy TransformComponent
    if (source.HasComponent<TransformComponent>()) {
        auto& srcTransform = source.GetComponent<TransformComponent>();
        auto& newTransform = newEntity.AddComponent<TransformComponent>();
        newTransform.localPosition = srcTransform.localPosition;
        newTransform.localRotation = srcTransform.localRotation;
        newTransform.localScale = srcTransform.localScale;
    }
    
    // Add HierarchyComponent (but don't copy parent/children)
    if (source.HasComponent<HierarchyComponent>()) {
        auto& srcHierarchy = source.GetComponent<HierarchyComponent>();
        auto& newHierarchy = newEntity.AddComponent<HierarchyComponent>();
        newHierarchy.parent = srcHierarchy.parent; // Keep same parent
    }
    
    // Copy MeshFilterComponent
    if (source.HasComponent<MeshFilterComponent>()) {
        auto& srcMesh = source.GetComponent<MeshFilterComponent>();
        auto& newMesh = newEntity.AddComponent<MeshFilterComponent>();
        newMesh.MeshHandle = srcMesh.MeshHandle;
        newMesh.mesh = srcMesh.mesh;
    }
    
    return newEntity;
}

Entity SceneHierarchyPanel::CreateChildEntity(Entity parent) {
    if (!parent || !m_Context) return {};
    
    // Create new entity
    EntityID childID = m_Context->CreateEntity();
    Entity child{ childID, m_Context };
    
    // Add components
    child.AddComponent<TagComponent>("Child Entity");
    child.AddComponent<TransformComponent>();
    
    // Set up hierarchy
    auto& childHierarchy = child.AddComponent<HierarchyComponent>();
    childHierarchy.parent = parent;
    
    // Add to parent's children list
    if (parent.HasComponent<HierarchyComponent>()) {
        auto& parentHierarchy = parent.GetComponent<HierarchyComponent>();
        
        if (parentHierarchy.firstChild == 0) {
            // First child
            parentHierarchy.firstChild = childID;
        } else {
            // Find last sibling and append
            EntityID lastSiblingID = parentHierarchy.firstChild;
            Entity lastSibling{ lastSiblingID, m_Context };
            
            while (lastSibling.HasComponent<HierarchyComponent>()) {
                auto& siblingHierarchy = lastSibling.GetComponent<HierarchyComponent>();
                if (siblingHierarchy.nextSibling == 0) {
                    siblingHierarchy.nextSibling = childID;
                    break;
                }
                lastSiblingID = siblingHierarchy.nextSibling;
                lastSibling = Entity{ lastSiblingID, m_Context };
            }
        }
    }
    
    return child;
}

} // namespace MyEngine
