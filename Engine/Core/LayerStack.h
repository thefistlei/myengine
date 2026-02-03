/******************************************************************************
 * File: LayerStack.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Layer stack management for application layers
 ******************************************************************************/

#pragma once

#include "Layer.h"
#include <vector>
#include <memory>

namespace MyEngine {

/**
 * @brief Manages a stack of layers
 * 
 * Layers are processed in order:
 * - Regular layers are added to the bottom of the stack
 * - Overlays are added to the top of the stack
 * 
 * Update order: bottom to top
 * Event order: top to bottom (so overlays can intercept events first)
 */
class LayerStack {
public:
    LayerStack() = default;
    ~LayerStack();
    
    /**
     * @brief Push layer to the stack (before overlays)
     */
    void PushLayer(std::shared_ptr<Layer> layer);
    
    /**
     * @brief Push overlay to the top of the stack
     */
    void PushOverlay(std::shared_ptr<Layer> overlay);
    
    /**
     * @brief Remove layer from stack
     */
    void PopLayer(std::shared_ptr<Layer> layer);
    
    /**
     * @brief Remove overlay from stack
     */
    void PopOverlay(std::shared_ptr<Layer> overlay);
    
    /**
     * @brief Get all layers
     */
    std::vector<std::shared_ptr<Layer>>& GetLayers() { return m_Layers; }
    const std::vector<std::shared_ptr<Layer>>& GetLayers() const { return m_Layers; }
    
    // Iterators for range-based for loops
    std::vector<std::shared_ptr<Layer>>::iterator begin() { return m_Layers.begin(); }
    std::vector<std::shared_ptr<Layer>>::iterator end() { return m_Layers.end(); }
    std::vector<std::shared_ptr<Layer>>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
    std::vector<std::shared_ptr<Layer>>::reverse_iterator rend() { return m_Layers.rend(); }
    
    std::vector<std::shared_ptr<Layer>>::const_iterator begin() const { return m_Layers.begin(); }
    std::vector<std::shared_ptr<Layer>>::const_iterator end() const { return m_Layers.end(); }
    std::vector<std::shared_ptr<Layer>>::const_reverse_iterator rbegin() const { return m_Layers.rbegin(); }
    std::vector<std::shared_ptr<Layer>>::const_reverse_iterator rend() const { return m_Layers.rend(); }
    
private:
    std::vector<std::shared_ptr<Layer>> m_Layers;
    unsigned int m_LayerInsertIndex = 0; // Index where regular layers end and overlays begin
};

} // namespace MyEngine
