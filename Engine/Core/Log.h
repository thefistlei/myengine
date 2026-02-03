/******************************************************************************
 * File: Log.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Engine logging system with multiple severity levels
 * Dependencies: <memory>, <string>
 ******************************************************************************/

#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace MyEngine {

/**
 * @brief Log severity levels
 */
enum class LogLevel {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Fatal = 5
};

/**
 * @brief Simple logger class (will integrate spdlog in future)
 */
class Logger {
public:
    Logger(const std::string& name) : m_Name(name), m_Level(LogLevel::Trace) {}

    void SetLevel(LogLevel level) { m_Level = level; }
    LogLevel GetLevel() const { return m_Level; }

    template<typename... Args>
    void Trace(const std::string& message, Args&&... args) {
        Log(LogLevel::Trace, message, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Debug(const std::string& message, Args&&... args) {
        Log(LogLevel::Debug, message, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Info(const std::string& message, Args&&... args) {
        Log(LogLevel::Info, message, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Warning(const std::string& message, Args&&... args) {
        Log(LogLevel::Warning, message, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Error(const std::string& message, Args&&... args) {
        Log(LogLevel::Error, message, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Fatal(const std::string& message, Args&&... args) {
        Log(LogLevel::Fatal, message, std::forward<Args>(args)...);
    }

private:
    template<typename... Args>
    void Log(LogLevel level, const std::string& message, Args&&... args) {
        if (level < m_Level) return;

        // Simple format implementation
        std::string formattedMessage = message;
        FormatMessage(formattedMessage, std::forward<Args>(args)...);

        // Get current time
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        struct tm timeinfo;
#ifdef _WIN32
        localtime_s(&timeinfo, &time_t);
#else
        localtime_r(&time_t, &timeinfo);
#endif

        char timebuf[64];
        std::strftime(timebuf, sizeof(timebuf), "%H:%M:%S", &timeinfo);

        // Format log level
        const char* levelStr = GetLevelString(level);
        const char* levelColor = GetLevelColor(level);

        // Print log
        std::cout << "[" << timebuf << "." 
                  << std::setfill('0') << std::setw(3) << ms.count()
                  << "] " << levelColor << "[" << levelStr << "] " 
                  << "\033[0m" << "[" << m_Name << "] "
                  << formattedMessage << std::endl;
    }

    void FormatMessage(std::string& fmt) {
        // Base case: no more arguments
    }

    template<typename T, typename... Args>
    void FormatMessage(std::string& fmt, T&& first, Args&&... rest) {
        size_t pos = fmt.find("{}");
        if (pos != std::string::npos) {
            std::stringstream ss;
            ss << first;
            fmt.replace(pos, 2, ss.str());
            FormatMessage(fmt, std::forward<Args>(rest)...);
        }
    }

    const char* GetLevelString(LogLevel level) const {
        switch (level) {
            case LogLevel::Trace:   return "TRACE";
            case LogLevel::Debug:   return "DEBUG";
            case LogLevel::Info:    return "INFO ";
            case LogLevel::Warning: return "WARN ";
            case LogLevel::Error:   return "ERROR";
            case LogLevel::Fatal:   return "FATAL";
            default: return "UNKN ";
        }
    }

    const char* GetLevelColor(LogLevel level) const {
        switch (level) {
            case LogLevel::Trace:   return "\033[37m";  // White
            case LogLevel::Debug:   return "\033[36m";  // Cyan
            case LogLevel::Info:    return "\033[32m";  // Green
            case LogLevel::Warning: return "\033[33m";  // Yellow
            case LogLevel::Error:   return "\033[31m";  // Red
            case LogLevel::Fatal:   return "\033[35m";  // Magenta
            default: return "\033[0m";
        }
    }

    std::string m_Name;
    LogLevel m_Level;
};

/**
 * @brief Global log manager
 */
class Log {
public:
    static void Init() {
        s_CoreLogger = std::make_shared<Logger>("ENGINE");
        s_ClientLogger = std::make_shared<Logger>("APP");
    }

    static std::shared_ptr<Logger>& GetCoreLogger() { return s_CoreLogger; }
    static std::shared_ptr<Logger>& GetClientLogger() { return s_ClientLogger; }

private:
    static std::shared_ptr<Logger> s_CoreLogger;
    static std::shared_ptr<Logger> s_ClientLogger;
};

} // namespace MyEngine

// Core logging macros (for engine internal use)
#define ENGINE_TRACE(...)    ::MyEngine::Log::GetCoreLogger()->Trace(__VA_ARGS__)
#define ENGINE_DEBUG(...)    ::MyEngine::Log::GetCoreLogger()->Debug(__VA_ARGS__)
#define ENGINE_INFO(...)     ::MyEngine::Log::GetCoreLogger()->Info(__VA_ARGS__)
#define ENGINE_WARN(...)     ::MyEngine::Log::GetCoreLogger()->Warning(__VA_ARGS__)
#define ENGINE_ERROR(...)    ::MyEngine::Log::GetCoreLogger()->Error(__VA_ARGS__)
#define ENGINE_FATAL(...)    ::MyEngine::Log::GetCoreLogger()->Fatal(__VA_ARGS__)

// Client logging macros (for application use)
#define APP_TRACE(...)       ::MyEngine::Log::GetClientLogger()->Trace(__VA_ARGS__)
#define APP_DEBUG(...)       ::MyEngine::Log::GetClientLogger()->Debug(__VA_ARGS__)
#define APP_INFO(...)        ::MyEngine::Log::GetClientLogger()->Info(__VA_ARGS__)
#define APP_WARN(...)        ::MyEngine::Log::GetClientLogger()->Warning(__VA_ARGS__)
#define APP_ERROR(...)       ::MyEngine::Log::GetClientLogger()->Error(__VA_ARGS__)
#define APP_FATAL(...)       ::MyEngine::Log::GetClientLogger()->Fatal(__VA_ARGS__)
