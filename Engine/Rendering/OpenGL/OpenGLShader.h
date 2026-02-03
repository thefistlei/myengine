/******************************************************************************
 * File: OpenGLShader.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: OpenGL shader implementation
 ******************************************************************************/

#pragma once

#include "Rendering/Shader.h"
#include <cstdint>

namespace MyEngine {

/**
 * @brief OpenGL implementation of the shader
 */
class OpenGLShader : public Shader {
public:
    OpenGLShader(const std::string& filepath);
    OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
    virtual ~OpenGLShader();
    
    void Bind() const override;
    void Unbind() const override;
    
    void SetInt(const std::string& name, int value) override;
    void SetBool(const std::string& name, bool value) override;
    void SetFloat(const std::string& name, float value) override;
    void SetFloat3(const std::string& name, const Vec3& value) override;
    void SetFloat4(const std::string& name, const Vec4& value) override;
    void SetMat4(const std::string& name, const Mat4& value) override;
    
    const std::string& GetName() const override { return m_Name; }
    
    void UploadUniformInt(const std::string& name, int value);
    void UploadUniformFloat(const std::string& name, float value);
    void UploadUniformFloat2(const std::string& name, const Vec2& value);
    void UploadUniformFloat3(const std::string& name, const Vec3& value);
    void UploadUniformFloat4(const std::string& name, const Vec4& value);
    void UploadUniformMat3(const std::string& name, const Mat3& matrix);
    void UploadUniformMat4(const std::string& name, const Mat4& matrix);
    
private:
    std::string ReadFile(const std::string& filepath);
    std::unordered_map<uint32_t, std::string> PreProcess(const std::string& source);
    void Compile(const std::unordered_map<uint32_t, std::string>& shaderSources);
    
private:
    uint32_t m_RendererID;
    std::string m_Name;
};

} // namespace MyEngine
