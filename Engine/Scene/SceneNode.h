/******************************************************************************
 * File: SceneNode.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Editor-only scene graph node for hierarchy visualization
 *              Not used in runtime performance-critical paths!
 ******************************************************************************/

#pragma once

#include "ECS/Registry.h"
#include <string>
#include <vector>
#include <memory>

namespace MyEngine {

/**
 * @brief Scene node for editor hierarchy tree display
 * 
 * IMPORTANT: This is EDITOR-ONLY!
 * - Does NOT store transform data (use ECS TransformComponent)
 * - Does NOT participate in runtime updates (use TransformSystem)
 * - Only for: hierarchy visualization, drag & drop, selection, naming
 * 
 * The actual runtime parent-child relationships are stored in HierarchyComponent (ECS)
 */
class SceneNode {
public:
    SceneNode(EntityID entityID, const std::string& name = "Entity")
        : m_EntityID(entityID), m_Name(name) {}
    
    // Entity reference
    EntityID GetEntityID() const { return m_EntityID; }
    
    // Editor properties
    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }
    
    bool IsExpanded() const { return m_EditorExpanded; }
    void SetExpanded(bool expanded) { m_EditorExpanded = expanded; }
    
    bool IsVisible() const { return m_EditorVisible; }
    void SetVisible(bool visible) { m_EditorVisible = visible; }
    
    // Editor hierarchy (for UI tree display)
    void AddChild(std::shared_ptr<SceneNode> child) {
        m_Children.push_back(child);
        child->m_Parent = this;
    }
    
    void RemoveChild(std::shared_ptr<SceneNode> child) {
        auto it = std::find(m_Children.begin(), m_Children.end(), child);
        if (it != m_Children.end()) {
            (*it)->m_Parent = nullptr;
            m_Children.erase(it);
        }
    }
    
    const std::vector<std::shared_ptr<SceneNode>>& GetChildren() const {
        return m_Children;
    }
    
    SceneNode* GetParent() const { return m_Parent; }
    
private:
    EntityID m_EntityID;                                    // Reference to ECS entity
    std::string m_Name;                                     // Display name in editor
    
    // Editor-only properties
    bool m_EditorExpanded = true;                           // Expanded in hierarchy tree
    bool m_EditorVisible = true;                            // Visible in editor
    
    // Editor tree structure (NOT runtime hierarchy!)
    SceneNode* m_Parent = nullptr;
    std::vector<std::shared_ptr<SceneNode>> m_Children;
};

/**
 * @brief Scene graph manager (editor utility)
 * Maps EntityID <-> SceneNode for editor operations
 */
class SceneGraph {
public:
    std::shared_ptr<SceneNode> CreateNode(EntityID entityID, const std::string& name = "Entity") {
        auto node = std::make_shared<SceneNode>(entityID, name);
        m_EntityToNode[entityID] = node;
        return node;
    }
    
    std::shared_ptr<SceneNode> GetNode(EntityID entityID) const {
        auto it = m_EntityToNode.find(entityID);
        return (it != m_EntityToNode.end()) ? it->second : nullptr;
    }
    
    void RemoveNode(EntityID entityID) {
        m_EntityToNode.erase(entityID);
    }
    
    const std::vector<std::shared_ptr<SceneNode>>& GetRootNodes() const {
        return m_RootNodes;
    }
    
    void AddRootNode(std::shared_ptr<SceneNode> node) {
        m_RootNodes.push_back(node);
    }
    
private:
    std::unordered_map<EntityID, std::shared_ptr<SceneNode>> m_EntityToNode;
    std::vector<std::shared_ptr<SceneNode>> m_RootNodes;
};

} // namespace MyEngine
