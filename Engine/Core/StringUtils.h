/******************************************************************************
 * File: StringUtils.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: String utility functions for common operations
 * Dependencies: <string>, <string_view>, <vector>, <algorithm>
 ******************************************************************************/

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <cctype>

namespace MyEngine {
namespace StringUtils {

/**
 * @brief Convert string to lowercase
 */
inline std::string ToLower(std::string_view str) {
    std::string result(str);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

/**
 * @brief Convert string to uppercase
 */
inline std::string ToUpper(std::string_view str) {
    std::string result(str);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

/**
 * @brief Trim whitespace from the left side
 */
inline std::string TrimLeft(std::string_view str) {
    std::string result(str);
    result.erase(result.begin(), std::find_if(result.begin(), result.end(),
                 [](unsigned char c) { return !std::isspace(c); }));
    return result;
}

/**
 * @brief Trim whitespace from the right side
 */
inline std::string TrimRight(std::string_view str) {
    std::string result(str);
    result.erase(std::find_if(result.rbegin(), result.rend(),
                 [](unsigned char c) { return !std::isspace(c); }).base(), result.end());
    return result;
}

/**
 * @brief Trim whitespace from both sides
 */
inline std::string Trim(std::string_view str) {
    return TrimLeft(TrimRight(str));
}

/**
 * @brief Split string by delimiter
 * 
 * @param str String to split
 * @param delimiter Delimiter character
 * @return Vector of substrings
 */
inline std::vector<std::string> Split(std::string_view str, char delimiter) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while (end != std::string_view::npos) {
        result.emplace_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    
    result.emplace_back(str.substr(start));
    return result;
}

/**
 * @brief Check if string starts with prefix
 */
inline bool StartsWith(std::string_view str, std::string_view prefix) {
    return str.size() >= prefix.size() && 
           str.compare(0, prefix.size(), prefix) == 0;
}

/**
 * @brief Check if string ends with suffix
 */
inline bool EndsWith(std::string_view str, std::string_view suffix) {
    return str.size() >= suffix.size() && 
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/**
 * @brief Check if string contains substring
 */
inline bool Contains(std::string_view str, std::string_view substr) {
    return str.find(substr) != std::string_view::npos;
}

/**
 * @brief Replace all occurrences of a substring
 */
inline std::string ReplaceAll(std::string_view str, std::string_view from, std::string_view to) {
    std::string result(str);
    size_t pos = 0;
    
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    
    return result;
}

} // namespace StringUtils
} // namespace MyEngine
