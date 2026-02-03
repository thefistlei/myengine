/******************************************************************************
 * File: ImGuiLayer.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: ImGui layer implementation
 ******************************************************************************/

#include "ImGuiLayer.h"
#include "Log.h"
#include "Application.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

namespace MyEngine {

ImGuiLayer::ImGuiLayer()
    : Layer("ImGuiLayer")
{
}

ImGuiLayer::~ImGuiLayer() {
}

void ImGuiLayer::OnAttach() {
    ENGINE_INFO("ImGuiLayer: Initializing ImGui");
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    
    // Enable keyboard navigation
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Note: Docking and ViewportsEnable require ImGui docking branch
    // Uncomment these if using docking branch:
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // When viewports are enabled, tweak WindowRounding/WindowBg
    // so platform windows can look identical to regular ones
    // ImGuiStyle& style = ImGui::GetStyle();
    // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    //     style.WindowRounding = 0.0f;
    //     style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    // }
    
    // Get GLFW window from Application
    Application& app = Application::Get();
    GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
    
    ENGINE_INFO("ImGuiLayer: Initialized successfully");
}

void ImGuiLayer::OnDetach() {
    ENGINE_INFO("ImGuiLayer: Shutting down ImGui");
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::OnUpdate(float deltaTime) {
    // ImGui update is handled in Begin/End
}

void ImGuiLayer::OnEvent(Event& event) {
    // ImGui event handling is done automatically by ImGui_ImplGlfw
    // We can block events here if needed
    ImGuiIO& io = ImGui::GetIO();
    
    // Example: Block mouse events when ImGui wants to capture mouse
    // event.Handled |= event.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
    
    // Example: Block keyboard events when ImGui wants to capture keyboard
    // event.Handled |= event.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
}

void ImGuiLayer::Begin() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::End() {
    ImGuiIO& io = ImGui::GetIO();
    Application& app = Application::Get();
    io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), 
                           (float)app.GetWindow().GetHeight());
    
    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // Update and Render additional Platform Windows
    // (Requires ImGui docking branch)
    // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    //     GLFWwindow* backup_current_context = glfwGetCurrentContext();
    //     ImGui::UpdatePlatformWindows();
    //     ImGui::RenderPlatformWindowsDefault();
    //     glfwMakeContextCurrent(backup_current_context);
    // }
}

} // namespace MyEngine
