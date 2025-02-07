#pragma once
#include <queue>
#include <thread>


class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads);

    void submit(const std::function<void()>& task);
    void pause();
    void resume();
    void stopNow();
    void shutdown();

    ~ThreadPool();

private:
    std::queue<std::function<void()>> _taskQueue;
    std::vector<std::thread> _workers;
    std::thread _manager;

    void executionCycle();
    void bufferingCycle();

    std::condition_variable _cv;
    std::mutex _mutex;

    std::atomic<bool> _stopFlag{false};
    std::atomic<bool> _immediateStopFlag{false};
    std::atomic<bool> _pauseFlag{false};
    std::atomic<bool> _executionPhaseFlag{false};

    // stats
    std::atomic<size_t> totalThreadsCreated{0};
    std::atomic<size_t> totalTasksExecuted{0};
    std::atomic<size_t> totalExecutionTime{0};
    std::atomic<size_t> totalWaitingTime{0};
    std::atomic<int> waitEventCount{0};
    std::atomic<size_t> totalQueueLength{0};
    std::atomic<size_t> queueCheckCount{0};
    void printStats() const;
};

