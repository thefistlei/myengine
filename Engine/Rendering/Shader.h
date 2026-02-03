/******************************************************************************
 * File: Shader.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Shader abstraction interface
 ******************************************************************************/

#pragma once

#include <string>
#include <unordered_map>
#include "Math/MathTypes.h"

namespace MyEngine {

/**
 * @brief Shader program interface
 */
class Shader {
public:
    virtual ~Shader() = default;
    
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    
    virtual void SetInt(const std::string& name, int value) = 0;
    virtual void SetBool(const std::string& name, bool value) = 0;
    virtual void SetFloat(const std::string& name, float value) = 0;
    virtual void SetFloat3(const std::string& name, const Vec3& value) = 0;
    virtual void SetFloat4(const std::string& name, const Vec4& value) = 0;
    virtual void SetMat4(const std::string& name, const Mat4& value) = 0;
    
    virtual const std::string& GetName() const = 0;
    
    static Shader* Create(const std::string& filepath);
    static Shader* Create(const std::string& name, 
                          const std::string& vertexSrc, 
                          const std::string& fragmentSrc);
};

/**
 * @brief Shader library for managing multiple shaders
 */
class ShaderLibrary {
public:
    void Add(const std::string& name, Shader* shader);
    void Add(Shader* shader);
    Shader* Load(const std::string& filepath);
    Shader* Load(const std::string& name, const std::string& filepath);
    
    Shader* Get(const std::string& name);
    bool Exists(const std::string& name) const;
    
private:
    std::unordered_map<std::string, Shader*> m_Shaders;
};

} // namespace MyEngine
