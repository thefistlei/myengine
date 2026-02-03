/******************************************************************************
 * File: OpenGLShader.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: OpenGL shader implementation
 ******************************************************************************/

#include "OpenGLShader.h"
#include "Core/Log.h"
#include "Platform/FileSystem.h"
#include <glad/gl.h>
#include <fstream>
#include <sstream>
#include <array>
#include <set>

namespace MyEngine {

static GLenum ShaderTypeFromString(const std::string& type) {
    if (type == "vertex")
        return GL_VERTEX_SHADER;
    if (type == "fragment" || type == "pixel")
        return GL_FRAGMENT_SHADER;
    
    return 0;
}

Shader* Shader::Create(const std::string& filepath) {
    return new OpenGLShader(filepath);
}

Shader* Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc) {
    return new OpenGLShader(name, vertexSrc, fragmentSrc);
}

OpenGLShader::OpenGLShader(const std::string& filepath) {
    std::string source = ReadFile(filepath);
    auto shaderSources = PreProcess(source);
    Compile(shaderSources);
    
    // Extract name from filepath
    auto lastSlash = filepath.find_last_of("/\\");
    lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
    auto lastDot = filepath.rfind('.');
    auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
    m_Name = filepath.substr(lastSlash, count);
}

OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
    : m_Name(name) {
    std::unordered_map<uint32_t, std::string> sources;
    sources[GL_VERTEX_SHADER] = vertexSrc;
    sources[GL_FRAGMENT_SHADER] = fragmentSrc;
    Compile(sources);
    ENGINE_INFO("Shader '{}' compiled successfully, ID={}", name, m_RendererID);
}

OpenGLShader::~OpenGLShader() {
    glDeleteProgram(m_RendererID);
}

std::string OpenGLShader::ReadFile(const std::string& filepath) {
    std::string result;
    std::ifstream in(filepath, std::ios::in | std::ios::binary);
    if (in) {
        in.seekg(0, std::ios::end);
        size_t size = in.tellg();
        if (size != -1) {
            result.resize(size);
            in.seekg(0, std::ios::beg);
            in.read(&result[0], size);
        }
        else {
            ENGINE_ERROR("Could not read from file '{0}'", filepath);
        }
    }
    else {
        ENGINE_ERROR("Could not open file '{0}'", filepath);
    }
    
    return result;
}

std::unordered_map<uint32_t, std::string> OpenGLShader::PreProcess(const std::string& source) {
    std::unordered_map<uint32_t, std::string> shaderSources;
    
    const char* typeToken = "#type";
    size_t typeTokenLength = strlen(typeToken);
    size_t pos = source.find(typeToken, 0);
    while (pos != std::string::npos) {
        size_t eol = source.find_first_of("\r\n", pos);
        size_t begin = pos + typeTokenLength + 1;
        std::string type = source.substr(begin, eol - begin);
        
        size_t nextLinePos = source.find_first_not_of("\r\n", eol);
        pos = source.find(typeToken, nextLinePos);
        
        shaderSources[ShaderTypeFromString(type)] = 
            (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
    }
    
    return shaderSources;
}

void OpenGLShader::Compile(const std::unordered_map<uint32_t, std::string>& shaderSources) {
    uint32_t program = glCreateProgram();
    std::array<uint32_t, 2> glShaderIDs;
    int glShaderIDIndex = 0;
    
    for (auto& kv : shaderSources) {
        uint32_t type = kv.first;
        const std::string& source = kv.second;
        
        uint32_t shader = glCreateShader(type);
        
        const char* sourceCStr = source.c_str();
        glShaderSource(shader, 1, &sourceCStr, 0);
        
        glCompileShader(shader);
        
        int isCompiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE) {
            int maxLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
            
            std::vector<char> infoLog(maxLength);
            glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
            
            glDeleteShader(shader);
            
            ENGINE_ERROR("{0}", infoLog.data());
            break;
        }
        
        glAttachShader(program, shader);
        glShaderIDs[glShaderIDIndex++] = shader;
    }
    
    m_RendererID = program;
    
    glLinkProgram(program);
    
    int isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
    if (isLinked == GL_FALSE) {
        int maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        
        std::vector<char> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
        
        glDeleteProgram(program);
        
        for (auto id : glShaderIDs)
            glDeleteShader(id);
        
        ENGINE_ERROR("{0}", infoLog.data());
        return;
    }
    
    for (auto id : glShaderIDs) {
        glDetachShader(program, id);
        glDeleteShader(id);
    }
}

void OpenGLShader::Bind() const {
    glUseProgram(m_RendererID);
}

void OpenGLShader::Unbind() const {
    glUseProgram(0);
}

void OpenGLShader::SetInt(const std::string& name, int value) {
    UploadUniformInt(name, value);
}

void OpenGLShader::SetBool(const std::string& name, bool value) {
    UploadUniformInt(name, value ? 1 : 0);
}

void OpenGLShader::SetFloat(const std::string& name, float value) {
    UploadUniformFloat(name, value);
}

void OpenGLShader::SetFloat3(const std::string& name, const Vec3& value) {
    UploadUniformFloat3(name, value);
}

void OpenGLShader::SetFloat4(const std::string& name, const Vec4& value) {
    UploadUniformFloat4(name, value);
}

void OpenGLShader::SetMat4(const std::string& name, const Mat4& value) {
    UploadUniformMat4(name, value);
}

void OpenGLShader::UploadUniformInt(const std::string& name, int value) {
    int location = glGetUniformLocation(m_RendererID, name.c_str());
    glUniform1i(location, value);
}

void OpenGLShader::UploadUniformFloat(const std::string& name, float value) {
    int location = glGetUniformLocation(m_RendererID, name.c_str());
    glUniform1f(location, value);
}

void OpenGLShader::UploadUniformFloat2(const std::string& name, const Vec2& value) {
    int location = glGetUniformLocation(m_RendererID, name.c_str());
    glUniform2f(location, value.x, value.y);
}

void OpenGLShader::UploadUniformFloat3(const std::string& name, const Vec3& value) {
    int location = glGetUniformLocation(m_RendererID, name.c_str());
    if (location == -1) {
        ENGINE_WARN("[OpenGLShader] Uniform '{}' not found in shader '{}'", name, m_Name);
        return;
    }
    glUniform3f(location, value.x, value.y, value.z);
}

void OpenGLShader::UploadUniformFloat4(const std::string& name, const Vec4& value) {
    int location = glGetUniformLocation(m_RendererID, name.c_str());
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void OpenGLShader::UploadUniformMat3(const std::string& name, const Mat3& matrix) {
    int location = glGetUniformLocation(m_RendererID, name.c_str());
    glUniformMatrix3fv(location, 1, GL_FALSE, matrix.m);
}

void OpenGLShader::UploadUniformMat4(const std::string& name, const Mat4& matrix) {
    int location = glGetUniformLocation(m_RendererID, name.c_str());
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, matrix.m);
    } else {
        static std::set<std::string> loggedMissing;
        if (loggedMissing.find(name) == loggedMissing.end()) {
            ENGINE_WARN("Uniform '{}' not found in shader '{}'", name, m_Name);
            loggedMissing.insert(name);
        }
    }
}

// ShaderLibrary implementation
void ShaderLibrary::Add(const std::string& name, Shader* shader) {
    m_Shaders[name] = shader;
}

void ShaderLibrary::Add(Shader* shader) {
    auto& name = shader->GetName();
    Add(name, shader);
}

Shader* ShaderLibrary::Load(const std::string& filepath) {
    auto shader = Shader::Create(filepath);
    Add(shader);
    return shader;
}

Shader* ShaderLibrary::Load(const std::string& name, const std::string& filepath) {
    auto shader = Shader::Create(filepath);
    Add(name, shader);
    return shader;
}

Shader* ShaderLibrary::Get(const std::string& name) {
    return m_Shaders[name];
}

bool ShaderLibrary::Exists(const std::string& name) const {
    return m_Shaders.find(name) != m_Shaders.end();
}

} // namespace MyEngine
