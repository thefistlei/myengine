/******************************************************************************
 * File: Layer.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Layer abstraction for application architecture
 *              Based on 3D_Engine_Editor_UI_Architecture_Guide.md
 ******************************************************************************/

#pragma once

#include "Event.h"
#include <string>

namespace MyEngine {

/**
 * @brief Base layer class for application organization
 * 
 * Layers allow organizing application logic into modular, stackable components.
 * Each layer can handle updates, events, and rendering independently.
 * 
 * Usage:
 * - Editor layers for UI panels and tools
 * - Game layers for gameplay logic
 * - Debug layers for profiling and visualization
 */
class Layer {
public:
    Layer(const std::string& name = "Layer") : m_DebugName(name) {}
    virtual ~Layer() = default;
    
    /**
     * @brief Called when layer is attached to application
     */
    virtual void OnAttach() {}
    
    /**
     * @brief Called when layer is detached from application
     */
    virtual void OnDetach() {}
    
    /**
     * @brief Called every frame for updates
     * @param deltaTime Time since last frame in seconds
     */
    virtual void OnUpdate(float deltaTime) {}
    
    /**
     * @brief Called for UI rendering (e.g., ImGui)
     */
    virtual void OnUIRender() {}
    
    /**
     * @brief Called when an event occurs
     * @param event Event to handle
     */
    virtual void OnEvent(Event& event) {}
    
    /**
     * @brief Get layer name (for debugging)
     */
    const std::string& GetName() const { return m_DebugName; }
    
protected:
    std::string m_DebugName;
};

} // namespace MyEngine
