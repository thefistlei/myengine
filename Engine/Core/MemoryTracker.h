/******************************************************************************
 * File: MemoryTracker.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Global memory usage tracking
 * Dependencies: Allocator.h
 ******************************************************************************/

#pragma once

#include "Allocator.h"
#include <atomic>
#include <string>

namespace MyEngine {

/**
 * @brief Global memory tracker for profiling
 */
class MemoryTracker {
public:
    /**
     * @brief Record allocation
     */
    static void RecordAllocation(size_t size, MemoryTag tag);

    /**
     * @brief Record deallocation
     */
    static void RecordDeallocation(size_t size, MemoryTag tag);

    /**
     * @brief Get total allocated memory across all tags
     */
    static size_t GetTotalAllocated();

    /**
     * @brief Get allocated memory for specific tag
     */
    static size_t GetTotalAllocatedByTag(MemoryTag tag);

    /**
     * @brief Print memory statistics to console
     */
    static void PrintStatistics();

    /**
     * @brief Reset all statistics
     */
    static void Reset();

    /**
     * @brief Get peak memory usage
     */
    static size_t GetPeakMemoryUsage();

    /**
     * @brief Get current memory usage
     */
    static size_t GetCurrentMemoryUsage();

    /**
     * @brief Check for memory leaks
     */
    static bool HasMemoryLeaks();

    /**
     * @brief Reset peak statistics
     */
    static void ResetPeakStats();

private:
    static std::atomic<size_t> s_TotalAllocated;
    static std::atomic<size_t> s_TaggedAllocations[static_cast<int>(MemoryTag::COUNT)];
    static std::atomic<size_t> s_PeakMemoryUsage;
    static std::atomic<size_t> s_AllocationCount;
    static std::atomic<size_t> s_DeallocationCount;
    
    static const char* GetTagName(MemoryTag tag);
};

} // namespace MyEngine
