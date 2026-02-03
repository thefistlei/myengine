/******************************************************************************
 * File: Window.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Platform-independent window abstraction
 * Dependencies: <string>, <functional>
 ******************************************************************************/

#pragma once

#include <string>
#include <functional>
#include <cstdint>

namespace MyEngine {

// Forward declaration
class Event;

/**
 * @brief Window creation properties
 */
struct WindowProps {
    std::string Title = "MyEngine";
    uint32_t Width = 1280;
    uint32_t Height = 720;
    bool VSync = true;
    bool Fullscreen = false;
};

/**
 * @brief Platform-independent window interface
 * 
 * Provides abstraction over native window creation and management.
 * Implementations are platform-specific (Windows, Linux, macOS).
 */
class Window {
public:
    using EventCallbackFn = std::function<void(Event&)>;
    
    virtual ~Window() = default;
    
    /**
     * @brief Update window state and poll events
     */
    virtual void OnUpdate() = 0;
    
    /**
     * @brief Swap front and back buffers (for OpenGL)
     */
    virtual void SwapBuffers() = 0;
    
    /**
     * @brief Get window width in pixels
     */
    virtual uint32_t GetWidth() const = 0;
    
    /**
     * @brief Get window height in pixels
     */
    virtual uint32_t GetHeight() const = 0;
    
    /**
     * @brief Get native window handle (platform-specific)
     * @return GLFWwindow* on GLFW, HWND on Win32, etc.
     */
    virtual void* GetNativeWindow() const = 0;
    
    /**
     * @brief Set event callback function
     */
    virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
    
    /**
     * @brief Enable/disable VSync
     */
    virtual void SetVSync(bool enabled) = 0;
    
    /**
     * @brief Check if VSync is enabled
     */
    virtual bool IsVSync() const = 0;
    
    /**
     * @brief Check if window should close
     */
    virtual bool ShouldClose() const = 0;
    
    /**
     * @brief Create a platform-specific window
     * @param props Window properties
     * @return Pointer to created window (caller owns)
     */
    static Window* Create(const WindowProps& props = WindowProps());
};

} // namespace MyEngine
