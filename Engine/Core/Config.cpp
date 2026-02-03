/******************************************************************************
 * File: Config.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Configuration system implementation
 ******************************************************************************/

#include "Config.h"
#include "Log.h"
#include "../Platform/FileSystem.h"
#include <iostream>
#include <sstream>

namespace MyEngine {

std::unordered_map<std::string, ConfigValue> Config::s_Config;

bool Config::LoadEngineConfig(const std::string& path) {
    ENGINE_INFO("Loading engine config: {}", path.c_str());

    // Simple INI-style parser (production would use proper parser)
    std::string content;
    if (!FileSystem::ReadFileToString(path, content)) {
        ENGINE_WARN("Failed to load engine config, using defaults");
        
        // Set defaults
        Set("window.width", 1280);
        Set("window.height", 720);
        Set("window.title", std::string("MyEngine"));
        Set("renderer.vsync", true);
        Set("renderer.api", std::string("OpenGL"));
        
        return false;
    }

    // Parse INI (simplified)
    std::istringstream stream(content);
    std::string line;
    while (std::getline(stream, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;

        // Parse key=value
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            // Try to parse as int, float, bool, or string
            if (value == "true" || value == "false") {
                Set(key, value == "true");
            } else if (value.find('.') != std::string::npos) {
                try {
                    Set(key, std::stof(value));
                } catch (...) {
                    Set(key, value);
                }
            } else {
                try {
                    Set(key, std::stoi(value));
                } catch (...) {
                    Set(key, value);
                }
            }
        }
    }

    ENGINE_INFO("Engine config loaded");
    return true;
}

bool Config::LoadProjectConfig(const std::string& path) {
    ENGINE_INFO("Loading project config: {}", path.c_str());
    // TODO: Implement JSON parsing
    // For now, set defaults
    Set("project.name", std::string("MyGame"));
    Set("project.version", std::string("0.1.0"));
    return true;
}

bool Config::LoadUserConfig(const std::string& path) {
    ENGINE_INFO("Loading user config: {}", path.c_str());
    // TODO: Implement JSON parsing
    // For now, set defaults
    Set("user.volume.master", 1.0f);
    Set("user.graphics.quality", std::string("High"));
    return true;
}

void Config::ParseCommandLine(int argc, char** argv) {
    ENGINE_INFO("Parsing command line arguments...");

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // Parse --key=value or --key value
        if (arg.substr(0, 2) == "--") {
            size_t pos = arg.find('=');
            if (pos != std::string::npos) {
                std::string key = arg.substr(2, pos - 2);
                std::string value = arg.substr(pos + 1);
                Set(key, value);
                ENGINE_INFO("Command line override: {} = {}", key.c_str(), value.c_str());
            } else if (i + 1 < argc) {
                std::string key = arg.substr(2);
                std::string value = argv[++i];
                Set(key, value);
                ENGINE_INFO("Command line override: {} = {}", key.c_str(), value.c_str());
            }
        }
    }
}

void Config::PrintAll() {
    std::cout << "\n========== Configuration ==========" << std::endl;
    for (const auto& pair : s_Config) {
        std::cout << "  " << pair.first << " = ";
        std::visit([](auto&& arg) { std::cout << arg; }, pair.second);
        std::cout << std::endl;
    }
    std::cout << "====================================" << std::endl;
}

} // namespace MyEngine
