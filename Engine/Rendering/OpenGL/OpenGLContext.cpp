/******************************************************************************
 * File: OpenGLContext.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: OpenGL graphics context implementation
 ******************************************************************************/

#include "OpenGLContext.h"
#include "Core/Log.h"

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace MyEngine {

OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
    : m_WindowHandle(windowHandle) {
    if (!m_WindowHandle) {
        ENGINE_ERROR("Window handle is null!");
    }
}

void OpenGLContext::Init() {
    glfwMakeContextCurrent(m_WindowHandle);
    
    int status = gladLoadGL(glfwGetProcAddress);
    if (!status) {
        ENGINE_ERROR("Failed to initialize Glad!");
        std::cout << "Failed to initialize Glad!" << std::endl;
    }
    
    std::cout << "OpenGL Info:" << std::endl;
    std::cout << "  Vendor:   " << (const char*)glGetString(GL_VENDOR) << std::endl;
    std::cout << "  Renderer: " << (const char*)glGetString(GL_RENDERER) << std::endl;
    std::cout << "  Version:  " << (const char*)glGetString(GL_VERSION) << std::endl;
    
    ENGINE_INFO("OpenGL Info:");
    ENGINE_INFO("  Vendor:   {}", (const char*)glGetString(GL_VENDOR));
    ENGINE_INFO("  Renderer: {}", (const char*)glGetString(GL_RENDERER));
    ENGINE_INFO("  Version:  {}", (const char*)glGetString(GL_VERSION));
}

void OpenGLContext::SwapBuffers() {
    glfwSwapBuffers(m_WindowHandle);
}

} // namespace MyEngine
