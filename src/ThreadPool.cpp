#include <iostream>

#include "ThreadPool.h"


ThreadPool::ThreadPool(const size_t numThreads) {
    std::cout << "Thread pool is created!" << std::endl;
    for (size_t i = 0; i < numThreads; ++i) {
        _workers.emplace_back(&ThreadPool::executionCycle, this);
    }
    _manager = std::thread(&ThreadPool::bufferingCycle, this);
}


void ThreadPool::submit(const std::function<void()>& task) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_stopFlag || _immediateStopFlag || _pauseFlag) {
        std::cout << "Task discarded (thread pool is shutting down or paused)." << std::endl;
        return;
    }
    if (_executionPhaseFlag) {
        std::cout << "Task discarded (there are still tasks in the queue to perform)." << std::endl;
        return;
    }
    _taskQueue.push(task);
}


void ThreadPool::pause() {
    _pauseFlag = true;
    std::cout << "Thread pool paused." << std::endl;
    _cv.notify_all();
}


void ThreadPool::resume() {
    _pauseFlag = false;
    std::cout << "Thread pool resumed." << std::endl;
    _cv.notify_all();
}


void ThreadPool::stopNow() {
    _immediateStopFlag = true;
    std::cout << "Stopping abruptly!" << std::endl;
    _cv.notify_all();

    for (std::thread& worker : _workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    if (_manager.joinable()) {
        _manager.join();
    }

    std::cout << "Thread pool stopped immediately." << std::endl;
    printStats();
}


void ThreadPool::shutdown() {
    _stopFlag = true;
    std::cout << "Completing current tasks if there are any...!" << std::endl;
    _cv.notify_all();

    for (std::thread& worker : _workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    if (_manager.joinable()) {
        _manager.join();
    }

    std::cout << "Thread pool shut down." << std::endl;
    printStats();
}


ThreadPool::~ThreadPool() {
    if (!_stopFlag && !_immediateStopFlag) {
        shutdown();
    }
}


void ThreadPool::executionCycle() {
    ++totalThreadsCreated;
    while (true) {
        if (_immediateStopFlag || (_stopFlag && _taskQueue.empty())) {
            break;
        }

        std::unique_lock<std::mutex> lock(_mutex);
        auto waitStart = std::chrono::high_resolution_clock::now();
        ++waitEventCount;
        _cv.wait(lock, [this] {  return (_executionPhaseFlag || _immediateStopFlag) && !_pauseFlag; });
        auto waitEnd = std::chrono::high_resolution_clock::now();
        totalWaitingTime += std::chrono::duration_cast<std::chrono::milliseconds>(waitEnd - waitStart).count();

        if (!_taskQueue.empty() && !_immediateStopFlag) {
            auto task = std::move(_taskQueue.front());
            _taskQueue.pop();
            lock.unlock();
            _cv.notify_one();

            auto taskStart = std::chrono::high_resolution_clock::now();
            task();
            auto taskEnd = std::chrono::high_resolution_clock::now();

            ++totalTasksExecuted;

            totalExecutionTime += std::chrono::duration_cast<std::chrono::milliseconds>(taskEnd - taskStart).count();

            lock.lock();
        }

        if (_taskQueue.empty()) {
            _executionPhaseFlag = false;
            _cv.notify_all();
        }
    }
}


void ThreadPool::bufferingCycle() {
    while (!_stopFlag && !_immediateStopFlag) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (!_executionPhaseFlag && !_pauseFlag) {
            std::cout << "Buffering tasks for 45 seconds..." << std::endl;
            _cv.wait_for(lock, std::chrono::seconds(45), [this] {return _stopFlag || _immediateStopFlag || _pauseFlag; });
            std::cout << "Buffering completed. Queue size after buffering: " << _taskQueue.size() << std::endl;

            if (_immediateStopFlag) {
                return;
            }

            _executionPhaseFlag = true;
            totalQueueLength += _taskQueue.size();
            ++queueCheckCount;
            _cv.notify_one();
        }
        _cv.wait(lock, [this] { return !_executionPhaseFlag || _stopFlag || _immediateStopFlag; });
    }
}


void ThreadPool::printStats() const {
    std::cout << "\n--- Performance Metrics ---\n";
    std::cout << "Threads Created: " << totalThreadsCreated << "\n";
    std::cout << "Tasks Executed: " << totalTasksExecuted << "\n";
    std::cout << "Average Waiting Time (ms): "
              << (waitEventCount ? totalWaitingTime / waitEventCount : 0) << "\n";
    std::cout << "Average Task Execution Time (ms): "
              << (totalTasksExecuted ? totalExecutionTime / totalTasksExecuted : 0) << "\n";
    std::cout << "Average Queue Length: "
              << (queueCheckCount ? totalQueueLength / queueCheckCount : 0) << "\n\n";

}
