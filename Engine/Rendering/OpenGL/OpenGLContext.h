/******************************************************************************
 * File: OpenGLContext.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: OpenGL graphics context implementation
 ******************************************************************************/

#pragma once

struct GLFWwindow;

namespace MyEngine {

/**
 * @brief OpenGL implementation of the graphics context
 */
class OpenGLContext {
public:
    /**
     * @brief Construct an OpenGL context for a window
     * @param windowHandle GLFWwindow pointer
     */
    OpenGLContext(GLFWwindow* windowHandle);
    
    /**
     * @brief Initialize OpenGL for the window
     */
    void Init();
    
    /**
     * @brief Swap the front and back buffers
     */
    void SwapBuffers();
    
private:
    GLFWwindow* m_WindowHandle;
};

} // namespace MyEngine
