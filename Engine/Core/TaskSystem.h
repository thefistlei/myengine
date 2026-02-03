/******************************************************************************
 * File: TaskSystem.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Work-stealing task system with task graph support
 ******************************************************************************/

#pragma once

#include <functional>
#include <atomic>
#include <vector>
#include <thread>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace MyEngine {

/**
 * @brief Task priority levels
 */
enum class TaskPriority {
    Background = 0,  // Low priority background tasks
    Low = 1,         // Low priority
    Normal = 2,      // Normal priority (default)
    High = 3         // High priority, executed ASAP
};

/**
 * @brief Task execution function
 */
using TaskFunction = std::function<void()>;

/**
 * @brief Task handle for tracking completion
 */
class TaskHandle {
public:
    TaskHandle() : m_Counter(std::make_shared<std::atomic<int>>(0)) {}
    
    void Wait() const {
        while (m_Counter && m_Counter->load(std::memory_order_acquire) > 0) {
            std::this_thread::yield();
        }
    }
    
    bool IsComplete() const {
        return !m_Counter || m_Counter->load(std::memory_order_acquire) == 0;
    }
    
    void Increment() {
        if (m_Counter) m_Counter->fetch_add(1, std::memory_order_release);
    }
    
    void Decrement() {
        if (m_Counter) m_Counter->fetch_sub(1, std::memory_order_release);
    }

private:
    std::shared_ptr<std::atomic<int>> m_Counter;
};

/**
 * @brief Task descriptor
 */
struct Task {
    TaskFunction Function;
    TaskPriority Priority = TaskPriority::Normal;
    TaskHandle Handle;
    
    Task() = default;
    Task(TaskFunction func, TaskPriority priority = TaskPriority::Normal)
        : Function(std::move(func)), Priority(priority) {}
};

/**
 * @brief Work-stealing task system
 */
class TaskSystem {
public:
    /**
     * @brief Initialize task system
     * @param numThreads Number of worker threads (0 = auto-detect)
     */
    static void Initialize(uint32_t numThreads = 0);
    
    /**
     * @brief Shutdown task system
     */
    static void Shutdown();
    
    /**
     * @brief Schedule a task
     * @param function Task function to execute
     * @param priority Task priority
     * @return Task handle for tracking
     */
    static TaskHandle Schedule(TaskFunction function, TaskPriority priority = TaskPriority::Normal);
    
    /**
     * @brief Schedule multiple tasks
     */
    static std::vector<TaskHandle> ScheduleBatch(const std::vector<TaskFunction>& functions, 
                                                   TaskPriority priority = TaskPriority::Normal);
    
    /**
     * @brief Wait for all tasks to complete
     */
    static void WaitForAll();
    
    /**
     * @brief Get number of worker threads
     */
    static uint32_t GetWorkerCount();
    
    /**
     * @brief Get singleton instance
     */
    static TaskSystem& Get();

private:
    TaskSystem() = default;
    ~TaskSystem() = default;
    
    void InitializeInternal(uint32_t numThreads);
    void ShutdownInternal();
    
    TaskHandle ScheduleInternal(TaskFunction function, TaskPriority priority);
    
    void WorkerThreadFunction(uint32_t threadIndex);
    bool TryStealTask(Task& outTask);
    bool TryPopTask(Task& outTask, uint32_t threadIndex);
    void PushTask(Task&& task, uint32_t threadIndex);
    
private:
    std::vector<std::thread> m_Workers;
    std::vector<std::deque<Task>> m_Queues;  // Per-thread queues
    std::vector<std::unique_ptr<std::mutex>> m_QueueMutexes;
    
    std::atomic<bool> m_Running{false};
    std::atomic<uint32_t> m_ActiveTasks{0};
    
    std::condition_variable m_WorkAvailable;
    std::mutex m_WaitMutex;
};

/**
 * @brief Fork-Join pattern helper
 */
class ParallelFor {
public:
    /**
     * @brief Execute function in parallel over range [0, count)
     * @param count Number of iterations
     * @param function Function to execute (takes index)
     * @param batchSize Items per task (0 = auto)
     */
    static void Execute(uint32_t count, 
                       std::function<void(uint32_t)> function,
                       uint32_t batchSize = 0);
};

} // namespace MyEngine
