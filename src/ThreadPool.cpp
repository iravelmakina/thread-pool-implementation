#include <iostream>

#include "ThreadPool.h"


ThreadPool::ThreadPool(const size_t numThreads) {
    std::cout << "Thread pool is created!" << std::endl;
    for (size_t i = 0; i < numThreads; ++i) {
        _workers.emplace_back(&ThreadPool::worker, this);
    }
}


void ThreadPool::submit(const std::function<void()>& task) {
    std::lock_guard lock(_mutex);
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
    _cv.notify_all();
    std::cout << "Thread pool paused." << std::endl;

}


void ThreadPool::resume() {
    _pauseFlag = false;
    _cv.notify_all();
    std::cout << "Thread pool resumed." << std::endl;
}


void ThreadPool::shutdown() {
    std::cout << "Completing current tasks if there are any...!" << std::endl;
    _stopFlag = true;
    _cv.notify_all();

    for (std::thread& worker : _workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    std::cout << "Thread pool shut down." << std::endl;
    printStats();
}


void ThreadPool::stopNow() {
    std::cout << "Stopping abruptly!" << std::endl;
    _immediateStopFlag = true;
    _cv.notify_all();

    for (std::thread& worker : _workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    std::cout << "Thread pool stopped immediately." << std::endl;
    printStats();
}


void ThreadPool::worker() {
    ++totalThreadsCreated;
    while(true) {
        if (_immediateStopFlag || _stopFlag) {
            break;
        }
        if (!_executionPhaseFlag && !_pauseFlag) {
            printf("Buffering tasks for 45 seconds...");
            auto start = std::chrono::steady_clock::now();
            while (std::chrono::high_resolution_clock::now() - start < std::chrono::seconds(45)) {
                if (_pauseFlag || _stopFlag || _immediateStopFlag) {
                    break;
                }
            }
            std::cout << "Buffering completed. Queue size after buffering: " << _taskQueue.size() << std::endl;
            _executionPhaseFlag = true;
            _cv.notify_all();
        }

        if (!_taskQueue.empty()) {
            totalQueueLength += _taskQueue.size();
            ++queueCheckCount;
        }

        std::unique_lock lock(_mutex);
        auto waitStart = std::chrono::high_resolution_clock::now();
        _cv.wait(lock, [this] {  return !_taskQueue.empty() ||_immediateStopFlag || _stopFlag || _pauseFlag; });
        auto waitEnd = std::chrono::high_resolution_clock::now();
        totalWaitingTime += std::chrono::duration_cast<std::chrono::milliseconds>(waitEnd - waitStart).count();

        while (!_taskQueue.empty() && !_immediateStopFlag && !_pauseFlag) {
            auto taskStart = std::chrono::high_resolution_clock::now();
            std::function<void()> task = _taskQueue.front();
            _taskQueue.pop();
            _cv.notify_all();
            lock.unlock();
            task();
            auto taskEnd = std::chrono::high_resolution_clock::now();
            ++totalTasksExecuted;
            totalExecutionTime += std::chrono::duration_cast<std::chrono::milliseconds>(taskEnd - taskStart).count(); // Update total execution time
            lock.lock();
        }
        if (_taskQueue.empty()) {
            _executionPhaseFlag = false;
        }
    }
}


ThreadPool::~ThreadPool() {
    if (!_stopFlag && !_immediateStopFlag) {
        shutdown();
    }
}


void ThreadPool::printStats() const {
    std::cout << "\n--- Performance Metrics ---\n";
    std::cout << "Threads Created: " << totalThreadsCreated << "\n";
    std::cout << "Tasks Executed: " << totalTasksExecuted << "\n";
    std::cout << "Average Waiting Time (ms): "
              << (totalThreadsCreated ? totalWaitingTime / totalThreadsCreated : 0) << "\n";
    std::cout << "Average Task Execution Time (ms): "
              << (totalTasksExecuted ? totalExecutionTime / totalTasksExecuted : 0) << "\n";
    std::cout << "Average Queue Length: "
              << (queueCheckCount ? totalQueueLength / queueCheckCount : 0) << "\n";

}
