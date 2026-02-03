/******************************************************************************
 * File: FileSystem.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: FileSystem implementation
 ******************************************************************************/

#include "FileSystem.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace MyEngine {
namespace FileSystem {

bool ReadFileToString(const std::string& path, std::string& outContent) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    outContent = buffer.str();
    
    file.close();
    return true;
}

bool ReadFileToBuffer(const std::string& path, std::vector<char>& outBuffer) {
    std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return false;
    }
    
    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);
    
    outBuffer.resize(fileSize);
    file.read(outBuffer.data(), fileSize);
    
    file.close();
    return true;
}

bool WriteStringToFile(const std::string& path, const std::string& content) {
    std::ofstream file(path, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << path << std::endl;
        return false;
    }
    
    file << content;
    file.close();
    return true;
}

std::vector<std::string> ListFiles(const std::string& directory, bool recursive) {
    std::vector<std::string> files;
    
    if (!Exists(directory) || !IsDirectory(directory)) {
        std::cerr << "Directory does not exist: " << directory << std::endl;
        return files;
    }
    
    try {
        if (recursive) {
            for (const auto& entry : fs::recursive_directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path().string());
                }
            }
        } else {
            for (const auto& entry : fs::directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path().string());
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
    
    return files;
}

std::vector<std::string> ListFilesWithExtension(const std::string& directory, 
                                                 const std::string& extension,
                                                 bool recursive) {
    std::vector<std::string> files;
    std::vector<std::string> allFiles = ListFiles(directory, recursive);
    
    for (const auto& file : allFiles) {
        if (GetExtension(file) == extension) {
            files.push_back(file);
        }
    }
    
    return files;
}

} // namespace FileSystem
} // namespace MyEngine
