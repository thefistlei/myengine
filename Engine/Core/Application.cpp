/******************************************************************************
 * File: Application.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Application implementation with main loop
 ******************************************************************************/

#include "Application.h"
#include "Log.h"
#include "Config.h"
#include "Module.h"
#include "ImGuiLayer.h"
#include "../Platform/Timer.h"
#include "../Rendering/Renderer.h"
#include <iostream>
#include <glad/gl.h>

namespace MyEngine {

Application* Application::s_Instance = nullptr;

Application::Application(const std::string& name, EngineMode mode)
    : m_Mode(mode) {
    // Ensure singleton
    if (s_Instance) {
        ENGINE_ERROR("Application already exists!");
        return;
    }
    s_Instance = this;

    ENGINE_INFO("Application created: {} (Mode: {})", name.c_str(), EngineModeToString(mode));

    // Load configurations
    Config::LoadEngineConfig();
    Config::LoadProjectConfig();
    Config::LoadUserConfig();

    // Create window (skip in Server/Tool mode)
    if (mode != EngineMode::Server && mode != EngineMode::Tool) {
        WindowProps props;
        props.Title = name;
        props.Width = Config::Get<int>("window.width", 1280);
        props.Height = Config::Get<int>("window.height", 720);

        m_Window = std::unique_ptr<Window>(Window::Create(props));
        m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });
    }

    // Initialize modules
    ModuleRegistry::Get().InitializeAll();
    ENGINE_INFO("All modules initialized successfully");
    
    // Initialize Renderer
    Renderer::Init();
    ENGINE_INFO("Renderer initialized");
    
    // Create and push ImGui layer (only in Editor mode)
    if (mode == EngineMode::Editor) {
        m_ImGuiLayer = std::make_shared<ImGuiLayer>();
        PushOverlay(m_ImGuiLayer);
    }
}

Application::~Application() {
    // Shutdown renderer
    Renderer::Shutdown();
    ENGINE_INFO("Renderer shutdown");
    
    // Shutdown modules
    ModuleRegistry::Get().ShutdownAll();
    
    ENGINE_INFO("Application destroyed");
}

void Application::Run() {
    ENGINE_INFO("Application started");

    Timer frameTimer;
    m_LastFrameTime = 0.0f;

    while (m_Running) {
        float currentTime = frameTimer.Elapsed();
        float deltaTime = currentTime - m_LastFrameTime;
        m_LastFrameTime = currentTime;

        if (!m_Minimized) {
            // Clear the screen
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // Update layers
            for (auto& layer : m_LayerStack) {
                layer->OnUpdate(deltaTime);
            }
            
            // Update modules
            ModuleRegistry::Get().UpdateAll(deltaTime);
            
            // Render ImGui (only if ImGuiLayer exists)
            if (m_ImGuiLayer) {
                m_ImGuiLayer->Begin();
                
                // Render all layer UIs
                for (auto& layer : m_LayerStack) {
                    layer->OnUIRender();
                }
                
                m_ImGuiLayer->End();
            }
            
            // Swap buffers
            if (m_Window) {
                m_Window->SwapBuffers();
            }
        }

        // Poll events - skip in headless mode
        if (m_Window) {
            m_Window->OnUpdate();
        }
    }

    ENGINE_INFO("Application stopped");
}

void Application::OnEvent(Event& e) {
    EventDispatcher dispatcher(e);
    
    // Handle window events
    dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& event) {
        return OnWindowClose(event);
    });
    
    dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& event) {
        return OnWindowResize(event);
    });
    
    // Propagate events to layers (in reverse order)
    for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
        if (e.Handled)
            break;
        (*it)->OnEvent(e);
    }

    // Log unhandled events in debug
    if (!e.Handled) {
        ENGINE_TRACE("Event: {}", e.ToString().c_str());
    }
}

bool Application::OnWindowClose(WindowCloseEvent& e) {
    m_Running = false;
    ENGINE_INFO("Window close requested");
    return true;
}

bool Application::OnWindowResize(WindowResizeEvent& e) {
    if (e.GetWidth() == 0 || e.GetHeight() == 0) {
        m_Minimized = true;
        ENGINE_INFO("Window minimized");
        return false;
    }

    m_Minimized = false;
    ENGINE_INFO("Window resized: {}x{}", e.GetWidth(), e.GetHeight());
    
    // Resize viewport here (once rendering is implemented)
    
    return false;
}

void Application::PushLayer(std::shared_ptr<Layer> layer) {
    m_LayerStack.PushLayer(layer);
}

void Application::PushOverlay(std::shared_ptr<Layer> overlay) {
    m_LayerStack.PushOverlay(overlay);
}

} // namespace MyEngine
