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
    std::thread _executor;

    void worker();
    void startExecutionCycle();
    bool runAllowed() const;

    std::condition_variable _cv;
    std::mutex _mutex;

    std::atomic<bool> _stopFlag{false};
    std::atomic<bool> _immediateStopFlag{false};
    std::atomic<bool> _pauseFlag{false};
    std::atomic<bool> _executionPhaseFlag{false};

};

