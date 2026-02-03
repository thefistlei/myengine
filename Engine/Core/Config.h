/******************************************************************************
 * File: Config.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Configuration system (engine/project/user settings)
 ******************************************************************************/

#pragma once

#include <string>
#include <unordered_map>
#include <variant>

namespace MyEngine {

/**
 * @brief Configuration value type
 */
using ConfigValue = std::variant<int, float, bool, std::string>;

/**
 * @brief Configuration system
 */
class Config {
public:
    /**
     * @brief Load engine configuration (engine.ini)
     */
    static bool LoadEngineConfig(const std::string& path = "config/engine.ini");

    /**
     * @brief Load project configuration (project.json)
     */
    static bool LoadProjectConfig(const std::string& path = "config/project.json");

    /**
     * @brief Load user preferences (user_preferences.json)
     */
    static bool LoadUserConfig(const std::string& path = "config/user_preferences.json");

    /**
     * @brief Parse command line arguments
     */
    static void ParseCommandLine(int argc, char** argv);

    /**
     * @brief Get configuration value
     */
    template<typename T>
    static T Get(const std::string& key, const T& defaultValue = T()) {
        auto it = s_Config.find(key);
        if (it != s_Config.end()) {
            try {
                return std::get<T>(it->second);
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    /**
     * @brief Set configuration value
     */
    template<typename T>
    static void Set(const std::string& key, const T& value) {
        s_Config[key] = value;
    }

    /**
     * @brief Check if key exists
     */
    static bool Has(const std::string& key) {
        return s_Config.find(key) != s_Config.end();
    }

    /**
     * @brief Print all configuration values
     */
    static void PrintAll();

private:
    static std::unordered_map<std::string, ConfigValue> s_Config;
};

} // namespace MyEngine
