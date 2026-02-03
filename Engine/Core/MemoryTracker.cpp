/******************************************************************************
 * File: MemoryTracker.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Memory tracker implementation
 ******************************************************************************/

#include "MemoryTracker.h"
#include <iostream>
#include <iomanip>

namespace MyEngine {

std::atomic<size_t> MemoryTracker::s_TotalAllocated{0};
std::atomic<size_t> MemoryTracker::s_TaggedAllocations[static_cast<int>(MemoryTag::COUNT)] = {0};
std::atomic<size_t> MemoryTracker::s_PeakMemoryUsage{0};
std::atomic<size_t> MemoryTracker::s_AllocationCount{0};
std::atomic<size_t> MemoryTracker::s_DeallocationCount{0};

void MemoryTracker::RecordAllocation(size_t size, MemoryTag tag) {
    s_TotalAllocated.fetch_add(size, std::memory_order_relaxed);
    s_TaggedAllocations[static_cast<int>(tag)].fetch_add(size, std::memory_order_relaxed);
    s_AllocationCount.fetch_add(1, std::memory_order_relaxed);
    
    // Update peak
    size_t current = s_TotalAllocated.load(std::memory_order_relaxed);
    size_t peak = s_PeakMemoryUsage.load(std::memory_order_relaxed);
    while (current > peak && !s_PeakMemoryUsage.compare_exchange_weak(peak, current)) {
        // Retry if another thread updated peak
    }
}

void MemoryTracker::RecordDeallocation(size_t size, MemoryTag tag) {
    s_TotalAllocated.fetch_sub(size, std::memory_order_relaxed);
    s_TaggedAllocations[static_cast<int>(tag)].fetch_sub(size, std::memory_order_relaxed);
    s_DeallocationCount.fetch_add(1, std::memory_order_relaxed);
}

size_t MemoryTracker::GetTotalAllocated() {
    return s_TotalAllocated.load(std::memory_order_relaxed);
}

size_t MemoryTracker::GetTotalAllocatedByTag(MemoryTag tag) {
    return s_TaggedAllocations[static_cast<int>(tag)].load(std::memory_order_relaxed);
}

void MemoryTracker::PrintStatistics() {
    std::cout << "\n========== Memory Statistics ==========" << std::endl;
    std::cout << "Total Allocated: " << GetTotalAllocated() << " bytes" << std::endl;
    std::cout << "\nBreakdown by tag:" << std::endl;
    
    for (int i = 0; i < static_cast<int>(MemoryTag::COUNT); ++i) {
        MemoryTag tag = static_cast<MemoryTag>(i);
        size_t allocated = GetTotalAllocatedByTag(tag);
        if (allocated > 0) {
            std::cout << "  " << std::setw(15) << std::left << GetTagName(tag) 
                      << ": " << allocated << " bytes" << std::endl;
        }
    }
    
    std::cout << "========================================" << std::endl;
}

void MemoryTracker::Reset() {
    s_TotalAllocated.store(0, std::memory_order_relaxed);
    for (int i = 0; i < static_cast<int>(MemoryTag::COUNT); ++i) {
        s_TaggedAllocations[i].store(0, std::memory_order_relaxed);
    }
    s_AllocationCount.store(0, std::memory_order_relaxed);
    s_DeallocationCount.store(0, std::memory_order_relaxed);
}

size_t MemoryTracker::GetPeakMemoryUsage() {
    return s_PeakMemoryUsage.load(std::memory_order_relaxed);
}

size_t MemoryTracker::GetCurrentMemoryUsage() {
    return s_TotalAllocated.load(std::memory_order_relaxed);
}

bool MemoryTracker::HasMemoryLeaks() {
    size_t allocs = s_AllocationCount.load(std::memory_order_relaxed);
    size_t deallocs = s_DeallocationCount.load(std::memory_order_relaxed);
    return allocs != deallocs || GetCurrentMemoryUsage() > 0;
}

void MemoryTracker::ResetPeakStats() {
    s_PeakMemoryUsage.store(s_TotalAllocated.load(std::memory_order_relaxed), std::memory_order_relaxed);
}

const char* MemoryTracker::GetTagName(MemoryTag tag) {
    switch (tag) {
        case MemoryTag::Unknown:   return "Unknown";
        case MemoryTag::General:   return "General";
        case MemoryTag::Rendering: return "Rendering";
        case MemoryTag::Physics:   return "Physics";
        case MemoryTag::Audio:     return "Audio";
        case MemoryTag::Scripting: return "Scripting";
        case MemoryTag::ECS:       return "ECS";
        case MemoryTag::Resource:  return "Resource";
        case MemoryTag::Platform:  return "Platform";
        case MemoryTag::Core:      return "Core";
        default:                   return "Unknown";
    }
}

} // namespace MyEngine
