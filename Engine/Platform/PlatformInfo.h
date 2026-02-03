/******************************************************************************
 * File: PlatformInfo.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: System information query utilities
 * Dependencies: <string>
 ******************************************************************************/

#pragma once

#include <string>
#include <cstdint>

namespace MyEngine {
namespace PlatformInfo {

/**
 * @brief Operating System type
 */
enum class OSType {
    Unknown,
    Windows,
    Linux,
    MacOS,
    Android,
    iOS
};

/**
 * @brief CPU Architecture
 */
enum class CPUArchitecture {
    Unknown,
    x86,
    x64,
    ARM,
    ARM64
};

/**
 * @brief Get operating system type
 */
OSType GetOSType();

/**
 * @brief Get OS version string (e.g., "Windows 10.0.26100")
 */
std::string GetOSVersion();

/**
 * @brief Get CPU architecture
 */
CPUArchitecture GetCPUArchitecture();

/**
 * @brief Get CPU name/model (e.g., "Intel Core i7-9700K")
 */
std::string GetCPUName();

/**
 * @brief Get number of logical CPU cores
 */
uint32_t GetCPUCoreCount();

/**
 * @brief Get total system RAM in MB
 */
uint64_t GetSystemRAM();

/**
 * @brief Get available (free) RAM in MB
 */
uint64_t GetAvailableRAM();

/**
 * @brief Get GPU name/model (requires graphics context)
 */
std::string GetGPUName();

/**
 * @brief Get GPU VRAM in MB (requires graphics context)
 */
uint64_t GetGPUMemory();

/**
 * @brief Print all system information to console
 */
void PrintSystemInfo();

} // namespace PlatformInfo
} // namespace MyEngine
