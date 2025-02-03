#pragma once
#include <thread>

#include "TaskQueue.h"

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads, TaskQueue &taskQueue);

    void stopNow(); // Immediate shutdown (cancels all tasks)
    void shutdown(); // Graceful shutdown (waits for tasks to complete)

    ~ThreadPool();

private:
    std::atomic<bool> stopFlag{false};
    std::atomic<bool> immediateStopFlag{false};
    TaskQueue &taskQueue;
    std::vector<std::thread> workers;
    void worker() const;
};

