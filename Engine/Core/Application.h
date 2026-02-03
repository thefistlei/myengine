/******************************************************************************
 * File: Application.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Engine application base class with main loop
 * Dependencies: Window, Event, Log
 ******************************************************************************/

#pragma once

#include "../Platform/Window.h"
#include "Event.h"
#include "ApplicationEvent.h"
#include "EngineMode.h"
#include "LayerStack.h"
#include <memory>
#include <string>

namespace MyEngine {

/**
 * @brief Base application class
 * - Manages window and main loop
 * - Handles events
 * - Delta time tracking
 */
class Application {
public:
    Application(const std::string& name = "MyEngine", EngineMode mode = EngineMode::Game);
    virtual ~Application();

    /**
     * @brief Run main application loop
     */
    void Run();

    /**
     * @brief Handle events
     */
    void OnEvent(Event& e);

    /**
     * @brief Get window instance
     */
    Window& GetWindow() { return *m_Window; }

    /**
     * @brief Get singleton instance
     */
    static Application& Get() { return *s_Instance; }

    /**
     * @brief Get engine mode
     */
    EngineMode GetMode() const { return m_Mode; }
    
    /**
     * @brief Set engine mode (switch between Editor and Player mode)
     */
    void SetMode(EngineMode mode) { m_Mode = mode; }
    
    /**
     * @brief Push layer onto stack
     */
    void PushLayer(std::shared_ptr<Layer> layer);
    
    /**
     * @brief Push overlay onto stack
     */
    void PushOverlay(std::shared_ptr<Layer> overlay);

private:
    bool OnWindowClose(WindowCloseEvent& e);
    bool OnWindowResize(WindowResizeEvent& e);

private:
    std::unique_ptr<Window> m_Window;
    EngineMode m_Mode;
    bool m_Running = true;
    bool m_Minimized = false;
    float m_LastFrameTime = 0.0f;
    
    LayerStack m_LayerStack;
    std::shared_ptr<class ImGuiLayer> m_ImGuiLayer;

    static Application* s_Instance;
};

/**
 * @brief Create application function (defined by client)
 */
Application* CreateApplication();

} // namespace MyEngine
