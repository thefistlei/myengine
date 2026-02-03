/******************************************************************************
 * File: FileSystem.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Cross-platform file system utilities
 * Dependencies: <string>, <vector>, <filesystem> (C++17)
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace MyEngine {
namespace FileSystem {

namespace fs = std::filesystem;

/**
 * @brief Check if file exists
 */
inline bool Exists(const std::string& path) {
    return fs::exists(path);
}

/**
 * @brief Check if path is a directory
 */
inline bool IsDirectory(const std::string& path) {
    return fs::is_directory(path);
}

/**
 * @brief Check if path is a regular file
 */
inline bool IsFile(const std::string& path) {
    return fs::is_regular_file(path);
}

/**
 * @brief Get file size in bytes
 */
inline size_t GetFileSize(const std::string& path) {
    if (!Exists(path)) return 0;
    return static_cast<size_t>(fs::file_size(path));
}

/**
 * @brief Read entire file into string
 * @param path File path
 * @param outContent Output string
 * @return true if successful, false otherwise
 */
bool ReadFileToString(const std::string& path, std::string& outContent);

/**
 * @brief Read entire file into byte buffer
 * @param path File path
 * @param outBuffer Output buffer
 * @return true if successful, false otherwise
 */
bool ReadFileToBuffer(const std::string& path, std::vector<char>& outBuffer);

/**
 * @brief Write string to file
 * @param path File path
 * @param content Content to write
 * @return true if successful, false otherwise
 */
bool WriteStringToFile(const std::string& path, const std::string& content);

/**
 * @brief Get file extension (e.g., ".txt")
 */
inline std::string GetExtension(const std::string& path) {
    return fs::path(path).extension().string();
}

/**
 * @brief Get filename without extension
 */
inline std::string GetStem(const std::string& path) {
    return fs::path(path).stem().string();
}

/**
 * @brief Get filename with extension
 */
inline std::string GetFilename(const std::string& path) {
    return fs::path(path).filename().string();
}

/**
 * @brief Get parent directory path
 */
inline std::string GetParentPath(const std::string& path) {
    return fs::path(path).parent_path().string();
}

/**
 * @brief Create directory (including parent directories)
 * @return true if successful or already exists
 */
inline bool CreateDirectory(const std::string& path) {
    return fs::create_directories(path);
}

/**
 * @brief Get absolute path
 */
inline std::string GetAbsolutePath(const std::string& path) {
    return fs::absolute(path).string();
}

/**
 * @brief Get current working directory
 */
inline std::string GetCurrentDirectory() {
    return fs::current_path().string();
}

/**
 * @brief List all files in directory
 * @param directory Directory path
 * @param recursive Whether to search recursively
 * @return Vector of file paths
 */
std::vector<std::string> ListFiles(const std::string& directory, bool recursive = false);

/**
 * @brief List files with specific extension
 * @param directory Directory path
 * @param extension Extension filter (e.g., ".cpp")
 * @param recursive Whether to search recursively
 * @return Vector of file paths
 */
std::vector<std::string> ListFilesWithExtension(const std::string& directory, 
                                                 const std::string& extension,
                                                 bool recursive = false);

} // namespace FileSystem
} // namespace MyEngine
