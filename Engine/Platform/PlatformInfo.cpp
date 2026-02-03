/******************************************************************************
 * File: PlatformInfo.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: System information implementation
 ******************************************************************************/

#include "PlatformInfo.h"
#include <iostream>
#include <thread>

#ifdef _WIN32
    #include <windows.h>
    #include <sysinfoapi.h>
#endif

namespace MyEngine {
namespace PlatformInfo {

OSType GetOSType() {
#ifdef _WIN32
    return OSType::Windows;
#elif defined(__linux__)
    return OSType::Linux;
#elif defined(__APPLE__)
    return OSType::MacOS;
#elif defined(__ANDROID__)
    return OSType::Android;
#elif defined(__IOS__)
    return OSType::iOS;
#else
    return OSType::Unknown;
#endif
}

std::string GetOSVersion() {
#ifdef _WIN32
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    
    // Note: GetVersionEx is deprecated, but works for this demo
    // In production, use VerifyVersionInfo or WMI
    #pragma warning(push)
    #pragma warning(disable: 4996)
    if (GetVersionEx((OSVERSIONINFO*)&osvi)) {
        return "Windows " + std::to_string(osvi.dwMajorVersion) + 
               "." + std::to_string(osvi.dwMinorVersion) +
               "." + std::to_string(osvi.dwBuildNumber);
    }
    #pragma warning(pop)
#endif
    return "Unknown OS Version";
}

CPUArchitecture GetCPUArchitecture() {
#ifdef _WIN64
    return CPUArchitecture::x64;
#elif defined(_WIN32)
    return CPUArchitecture::x86;
#elif defined(__aarch64__) || defined(_M_ARM64)
    return CPUArchitecture::ARM64;
#elif defined(__arm__) || defined(_M_ARM)
    return CPUArchitecture::ARM;
#elif defined(__x86_64__) || defined(__amd64__)
    return CPUArchitecture::x64;
#elif defined(__i386__)
    return CPUArchitecture::x86;
#else
    return CPUArchitecture::Unknown;
#endif
}

std::string GetCPUName() {
#ifdef _WIN32
    // Simplified - would need CPUID or WMI for real CPU name
    return "x64 Processor";
#else
    return "Unknown CPU";
#endif
}

uint32_t GetCPUCoreCount() {
    return std::thread::hardware_concurrency();
}

uint64_t GetSystemRAM() {
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    return memInfo.ullTotalPhys / (1024 * 1024); // Convert to MB
#else
    return 0;
#endif
}

uint64_t GetAvailableRAM() {
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    return memInfo.ullAvailPhys / (1024 * 1024); // Convert to MB
#else
    return 0;
#endif
}

std::string GetGPUName() {
    // Requires OpenGL/Vulkan context - will implement in later steps
    return "Unknown GPU (requires graphics context)";
}

uint64_t GetGPUMemory() {
    // Requires OpenGL/Vulkan context - will implement in later steps
    return 0;
}

void PrintSystemInfo() {
    std::cout << "\n========== System Information ==========" << std::endl;
    
    OSType osType = GetOSType();
    std::cout << "OS: ";
    switch (osType) {
        case OSType::Windows: std::cout << "Windows"; break;
        case OSType::Linux: std::cout << "Linux"; break;
        case OSType::MacOS: std::cout << "macOS"; break;
        default: std::cout << "Unknown"; break;
    }
    std::cout << std::endl;
    
    std::cout << "OS Version: " << GetOSVersion() << std::endl;
    
    CPUArchitecture arch = GetCPUArchitecture();
    std::cout << "CPU Architecture: ";
    switch (arch) {
        case CPUArchitecture::x86: std::cout << "x86 (32-bit)"; break;
        case CPUArchitecture::x64: std::cout << "x64 (64-bit)"; break;
        case CPUArchitecture::ARM: std::cout << "ARM (32-bit)"; break;
        case CPUArchitecture::ARM64: std::cout << "ARM64 (64-bit)"; break;
        default: std::cout << "Unknown"; break;
    }
    std::cout << std::endl;
    
    std::cout << "CPU Cores: " << GetCPUCoreCount() << std::endl;
    std::cout << "Total RAM: " << GetSystemRAM() << " MB" << std::endl;
    std::cout << "Available RAM: " << GetAvailableRAM() << " MB" << std::endl;
    
    std::cout << "========================================" << std::endl;
}

} // namespace PlatformInfo
} // namespace MyEngine
