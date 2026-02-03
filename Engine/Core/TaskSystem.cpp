/******************************************************************************
 * File: TaskSystem.cpp
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Task system implementation
 ******************************************************************************/

#include "TaskSystem.h"
#include "Log.h"
#include <algorithm>
#include <random>

namespace MyEngine {

// Static members
static TaskSystem* s_Instance = nullptr;

void TaskSystem::Initialize(uint32_t numThreads) {
    if (!s_Instance) {
        s_Instance = new TaskSystem();
        s_Instance->InitializeInternal(numThreads);
    }
}

void TaskSystem::Shutdown() {
    if (s_Instance) {
        s_Instance->ShutdownInternal();
        delete s_Instance;
        s_Instance = nullptr;
    }
}

TaskHandle TaskSystem::Schedule(TaskFunction function, TaskPriority priority) {
    if (s_Instance) {
        return s_Instance->ScheduleInternal(std::move(function), priority);
    }
    return TaskHandle();
}

std::vector<TaskHandle> TaskSystem::ScheduleBatch(const std::vector<TaskFunction>& functions, 
                                                    TaskPriority priority) {
    std::vector<TaskHandle> handles;
    handles.reserve(functions.size());
    
    for (const auto& func : functions) {
        handles.push_back(Schedule(func, priority));
    }
    
    return handles;
}

void TaskSystem::WaitForAll() {
    if (s_Instance) {
        while (s_Instance->m_ActiveTasks.load(std::memory_order_acquire) > 0) {
            std::this_thread::yield();
        }
    }
}

uint32_t TaskSystem::GetWorkerCount() {
    return s_Instance ? static_cast<uint32_t>(s_Instance->m_Workers.size()) : 0;
}

TaskSystem& TaskSystem::Get() {
    return *s_Instance;
}

void TaskSystem::InitializeInternal(uint32_t numThreads) {
    if (numThreads == 0) {
        numThreads = std::thread::hardware_concurrency();
        if (numThreads > 1) numThreads -= 1;  // Leave one core for main thread
    }
    
    numThreads = std::max(1u, numThreads);
    
    ENGINE_INFO("Initializing TaskSystem with {} worker threads", numThreads);
    
    m_Running.store(true, std::memory_order_release);
    
    // Initialize queues
    m_Queues.resize(numThreads);
    m_QueueMutexes.reserve(numThreads);
    for (uint32_t i = 0; i < numThreads; ++i) {
        m_QueueMutexes.push_back(std::make_unique<std::mutex>());
    }
    
    // Spawn worker threads
    m_Workers.reserve(numThreads);
    for (uint32_t i = 0; i < numThreads; ++i) {
        m_Workers.emplace_back(&TaskSystem::WorkerThreadFunction, this, i);
    }
    
    ENGINE_INFO("TaskSystem initialized successfully");
}

void TaskSystem::ShutdownInternal() {
    ENGINE_INFO("Shutting down TaskSystem...");
    
    // Signal shutdown
    m_Running.store(false, std::memory_order_release);
    m_WorkAvailable.notify_all();
    
    // Wait for all workers
    for (auto& worker : m_Workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    m_Workers.clear();
    m_Queues.clear();
    m_QueueMutexes.clear();
    
    ENGINE_INFO("TaskSystem shut down");
}

TaskHandle TaskSystem::ScheduleInternal(TaskFunction function, TaskPriority priority) {
    Task task(std::move(function), priority);
    
    m_ActiveTasks.fetch_add(1, std::memory_order_release);
    task.Handle.Increment();
    
    TaskHandle handle = task.Handle;
    
    // Round-robin distribution to worker queues
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, static_cast<uint32_t>(m_Queues.size() - 1));
    uint32_t targetQueue = dist(gen);
    
    PushTask(std::move(task), targetQueue);
    m_WorkAvailable.notify_one();
    
    return handle;
}

void TaskSystem::WorkerThreadFunction(uint32_t threadIndex) {
    Task task;
    
    while (m_Running.load(std::memory_order_acquire)) {
        // Try to get task from own queue first
        if (TryPopTask(task, threadIndex)) {
            // Execute task
            if (task.Function) {
                task.Function();
            }
            
            task.Handle.Decrement();
            m_ActiveTasks.fetch_sub(1, std::memory_order_release);
            continue;
        }
        
        // Try to steal from other queues
        if (TryStealTask(task)) {
            // Execute stolen task
            if (task.Function) {
                task.Function();
            }
            
            task.Handle.Decrement();
            m_ActiveTasks.fetch_sub(1, std::memory_order_release);
            continue;
        }
        
        // No work available, wait
        std::unique_lock<std::mutex> lock(m_WaitMutex);
        m_WorkAvailable.wait_for(lock, std::chrono::milliseconds(1));
    }
}

bool TaskSystem::TryStealTask(Task& outTask) {
    // Try to steal from random queues
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    
    std::uniform_int_distribution<size_t> dist(0, m_Queues.size() - 1);
    
    for (size_t attempts = 0; attempts < m_Queues.size(); ++attempts) {
        size_t queueIndex = dist(gen);
        
        std::unique_lock<std::mutex> lock(*m_QueueMutexes[queueIndex], std::try_to_lock);
        if (lock.owns_lock() && !m_Queues[queueIndex].empty()) {
            // Steal from back (FIFO for stealing)
            outTask = std::move(m_Queues[queueIndex].back());
            m_Queues[queueIndex].pop_back();
            return true;
        }
    }
    
    return false;
}

bool TaskSystem::TryPopTask(Task& outTask, uint32_t threadIndex) {
    std::unique_lock<std::mutex> lock(*m_QueueMutexes[threadIndex], std::try_to_lock);
    if (lock.owns_lock() && !m_Queues[threadIndex].empty()) {
        // Pop from front (LIFO for own queue - better cache locality)
        outTask = std::move(m_Queues[threadIndex].front());
        m_Queues[threadIndex].pop_front();
        return true;
    }
    return false;
}

void TaskSystem::PushTask(Task&& task, uint32_t threadIndex) {
    std::lock_guard<std::mutex> lock(*m_QueueMutexes[threadIndex]);
    
    // Insert based on priority
    auto it = m_Queues[threadIndex].begin();
    while (it != m_Queues[threadIndex].end() && it->Priority >= task.Priority) {
        ++it;
    }
    m_Queues[threadIndex].insert(it, std::move(task));
}

// ParallelFor implementation
void ParallelFor::Execute(uint32_t count, 
                         std::function<void(uint32_t)> function,
                         uint32_t batchSize) {
    if (count == 0) return;
    
    uint32_t workerCount = TaskSystem::GetWorkerCount();
    if (workerCount == 0) {
        // No task system, execute serially
        for (uint32_t i = 0; i < count; ++i) {
            function(i);
        }
        return;
    }
    
    // Auto-calculate batch size
    if (batchSize == 0) {
        batchSize = std::max(1u, count / (workerCount * 4));
    }
    
    uint32_t numBatches = (count + batchSize - 1) / batchSize;
    std::vector<TaskHandle> handles;
    handles.reserve(numBatches);
    
    for (uint32_t batch = 0; batch < numBatches; ++batch) {
        uint32_t start = batch * batchSize;
        uint32_t end = std::min(start + batchSize, count);
        
        auto handle = TaskSystem::Schedule([=]() {
            for (uint32_t i = start; i < end; ++i) {
                function(i);
            }
        });
        
        handles.push_back(handle);
    }
    
    // Wait for all batches
    for (auto& handle : handles) {
        handle.Wait();
    }
}

} // namespace MyEngine
