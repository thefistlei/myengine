/******************************************************************************
 * File: WindowsWindow.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: GLFW window implementation
 ******************************************************************************/

#include "WindowsWindow.h"
#include "Core/ApplicationEvent.h"
#include <iostream>
#include <glad/gl.h>

// Note: GLFW will be included only if available
#ifdef MYENGINE_USE_GLFW
    #include <GLFW/glfw3.h>
#endif

namespace MyEngine {

static bool s_GLFWInitialized = false;

static void GLFWErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

Window* Window::Create(const WindowProps& props) {
    return new WindowsWindow(props);
}

WindowsWindow::WindowsWindow(const WindowProps& props) {
    Init(props);
}

WindowsWindow::~WindowsWindow() {
    Shutdown();
}

void WindowsWindow::Init(const WindowProps& props) {
#ifdef MYENGINE_USE_GLFW
    m_Data.Title = props.Title;
    m_Data.Width = props.Width;
    m_Data.Height = props.Height;
    m_Data.VSync = props.VSync;
    
    std::cout << "Creating window: " << props.Title 
              << " (" << props.Width << "x" << props.Height << ")" << std::endl;
    
    // Initialize GLFW
    if (!s_GLFWInitialized) {
        int success = glfwInit();
        if (!success) {
            std::cerr << "Failed to initialize GLFW!" << std::endl;
            return;
        }
        
        glfwSetErrorCallback(GLFWErrorCallback);
        s_GLFWInitialized = true;
    }
    
    // Set OpenGL hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Create window
    m_Window = glfwCreateWindow(
        static_cast<int>(props.Width),
        static_cast<int>(props.Height),
        m_Data.Title.c_str(),
        nullptr,
        nullptr
    );
    
    if (!m_Window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        return;
    }
    
    // Create graphics context
    m_Context = new OpenGLContext(m_Window);
    m_Context->Init();
    
    // Set user pointer to window data (for callbacks)
    glfwSetWindowUserPointer(m_Window, &m_Data);
    
    // Set callbacks
    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        data.Width = width;
        data.Height = height;
        glViewport(0, 0, width, height);
    });
    
    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        if (data.EventCallback) {
            WindowCloseEvent event;
            data.EventCallback(event);
        }
    });

    // Set VSync
    SetVSync(m_Data.VSync);
    
    std::cout << "Window created successfully!" << std::endl;
#else
    std::cout << "GLFW not available - using stub window" << std::endl;
    m_Window = nullptr;
    m_Data.Title = props.Title;
    m_Data.Width = props.Width;
    m_Data.Height = props.Height;
    m_Data.VSync = props.VSync;
#endif
}

void WindowsWindow::Shutdown() {
#ifdef MYENGINE_USE_GLFW
    if (m_Context) {
        delete m_Context;
        m_Context = nullptr;
    }
    if (m_Window) {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }
#endif
}

void WindowsWindow::OnUpdate() {
#ifdef MYENGINE_USE_GLFW
    if (m_Window) {
        glfwPollEvents();
    }
#endif
}

void WindowsWindow::SwapBuffers() {
#ifdef MYENGINE_USE_GLFW
    if (m_Context) {
        m_Context->SwapBuffers();
    }
#endif
}

void WindowsWindow::SetVSync(bool enabled) {
#ifdef MYENGINE_USE_GLFW
    if (enabled) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }
#endif
    m_Data.VSync = enabled;
}

bool WindowsWindow::ShouldClose() const {
#ifdef MYENGINE_USE_GLFW
    if (m_Window) {
        return glfwWindowShouldClose(m_Window);
    }
#endif
    return false;
}

} // namespace MyEngine
