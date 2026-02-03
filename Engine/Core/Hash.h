/******************************************************************************
 * File: Hash.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Hash functions for strings and compile-time hashing
 * Dependencies: <cstdint>, <string_view>
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <string_view>

namespace MyEngine {

/**
 * @brief FNV-1a Hash Algorithm (64-bit)
 * 
 * Fast and simple hash function for runtime string hashing.
 * FNV (Fowler-Noll-Vo) is a non-cryptographic hash function.
 * 
 * @param str String to hash
 * @return 64-bit hash value
 */
inline uint64_t HashString(std::string_view str) {
    constexpr uint64_t FNV_offset = 14695981039346656037ULL;
    constexpr uint64_t FNV_prime = 1099511628211ULL;
    
    uint64_t hash = FNV_offset;
    for (char c : str) {
        hash ^= static_cast<uint64_t>(c);
        hash *= FNV_prime;
    }
    return hash;
}

/**
 * @brief Compile-time String Hash (constexpr)
 * 
 * Allows hashing strings at compile time for use in switch statements
 * and other compile-time contexts.
 * 
 * @param str C-style string to hash
 * @param hash Initial hash value (internal use)
 * @return 64-bit hash value
 */
constexpr uint64_t HashStringCompileTime(const char* str, uint64_t hash = 14695981039346656037ULL) {
    return (*str == 0) ? hash : HashStringCompileTime(str + 1, (hash ^ static_cast<uint64_t>(*str)) * 1099511628211ULL);
}

/**
 * @brief String ID Macro for compile-time hashing
 * 
 * Usage:
 *   constexpr uint64_t id = STRING_ID("MyIdentifier");
 *   
 * This allows using strings as compile-time constants for fast comparisons.
 */
#define STRING_ID(str) (MyEngine::HashStringCompileTime(str))

/**
 * @brief Combine two hash values
 * 
 * Useful for creating hash values from multiple components.
 * Based on boost::hash_combine.
 */
inline uint64_t HashCombine(uint64_t seed, uint64_t hash) {
    return seed ^ (hash + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
}

} // namespace MyEngine
