/******************************************************************************
 * File: ImGuiLayer.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: ImGui layer for UI rendering
 *              Handles ImGui initialization, rendering, and cleanup
 ******************************************************************************/

#pragma once

#include "Layer.h"

namespace MyEngine {

/**
 * @brief ImGui Layer
 * 
 * Manages ImGui context and rendering:
 * - Initializes ImGui with GLFW and OpenGL3
 * - Handles per-frame ImGui setup (NewFrame, Render)
 * - Processes events for ImGui input
 * - Cleans up ImGui resources
 */
class ImGuiLayer : public Layer {
public:
    ImGuiLayer();
    virtual ~ImGuiLayer();
    
    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate(float deltaTime) override;
    virtual void OnEvent(Event& event) override;
    
    /**
     * @brief Begin ImGui frame
     * Call this before rendering any ImGui UI
     */
    void Begin();
    
    /**
     * @brief End ImGui frame and render
     * Call this after all ImGui UI has been submitted
     */
    void End();
    
private:
    float m_Time = 0.0f;
};

} // namespace MyEngine
