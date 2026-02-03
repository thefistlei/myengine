/******************************************************************************
 * File: WindowsWindow.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: GLFW-based window implementation (cross-platform)
 * Dependencies: GLFW, Window.h
 * Note: Named "WindowsWindow" but actually uses GLFW for cross-platform support
 ******************************************************************************/

#pragma once

#include "Window.h"
#include "Rendering/OpenGL/OpenGLContext.h"

struct GLFWwindow;

namespace MyEngine {

/**
 * @brief GLFW-based window implementation
 * 
 * Uses GLFW library for cross-platform window management.
 * Despite the name, this works on Windows, Linux, and macOS.
 */
class WindowsWindow : public Window {
public:
    WindowsWindow(const WindowProps& props);
    virtual ~WindowsWindow();
    
    void OnUpdate() override;
    void SwapBuffers() override;
    
    uint32_t GetWidth() const override { return m_Data.Width; }
    uint32_t GetHeight() const override { return m_Data.Height; }
    
    void* GetNativeWindow() const override { return m_Window; }
    
    void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
    void SetVSync(bool enabled) override;
    bool IsVSync() const override { return m_Data.VSync; }
    
    bool ShouldClose() const override;
    
private:
    void Init(const WindowProps& props);
    void Shutdown();
    
private:
    GLFWwindow* m_Window;
    OpenGLContext* m_Context;
    
    struct WindowData {
        std::string Title;
        uint32_t Width, Height;
        bool VSync;
        
        EventCallbackFn EventCallback;
    };
    
    WindowData m_Data;
};

} // namespace MyEngine
