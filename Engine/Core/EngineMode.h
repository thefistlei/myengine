/******************************************************************************
 * File: EngineMode.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Engine run modes (Game/Editor/Server/Tool)
 ******************************************************************************/

#pragma once

namespace MyEngine {

/**
 * @brief Engine运行模式
 */
enum class EngineMode {
    Game,       // 正常游戏模式
    Editor,     // 编辑器模式
    Server,     // 服务器模式（无渲染）
    Tool        // 命令行工具模式
};

/**
 * @brief Get engine mode as string
 */
inline const char* EngineModeToString(EngineMode mode) {
    switch (mode) {
        case EngineMode::Game:   return "Game";
        case EngineMode::Editor: return "Editor";
        case EngineMode::Server: return "Server";
        case EngineMode::Tool:   return "Tool";
        default:                 return "Unknown";
    }
}

} // namespace MyEngine
